/* LittleLaser
    Flash a $1 pen laser with a (controllable) temporal profile mimicking a Ti:Sapphire high-intensity laser

    With the arrival of each new external / internal trigger pulse:
      A global counter is incremented
      
    Built for a Teensy 4.1 board.
    Based on testing with a Teensy 4.1 board.

    Tested to work fine at repetition rates up to or even slightly beyond 1 kHz.

    PINS:
      23    |   Input, external trigger (optional; internally triggered by default)
      19    |   Output, reference trigger (optional usage for internal triggering is to bridge pins 19 and 23). User-configurable repetition rate.
      14     |   Output, Pen laser control (this PWM output goes to the base of a transistor which switches the actual laser)
      3     |   Output, green status LED, heartbeat. Flashes once per second reliably.
            
    Written using the other existing DolphinDAQ projects (PulseGenerator, BlinkSync, and Rosie) as computer code templates.

    Followed TeensyTimerTool tutorial at: https://forum.pjrc.com/threads/59112-TeensyTimerTool
    
    Written by Scott Feister on September 23, 2023.
*/

#include <Arduino.h>
#include <Chrono.h> //non-interrupt, low-precision timing
#include "scpi-def.h"
#include "TeensyTimerTool.h"
using namespace TeensyTimerTool;

#define REF_PIN 19 // reference / internal trigger pin
#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN 3
#define LASER_PIN 22 // Note, must avoid PWM clashes: https://github.com/luni64/TeensyTimerTool/wiki/Avoid-PWM-timer-clashes#teensy-41
#define MAIN_WIDTH_US 10 // (lower bound for) main-pulse width in microseconds
#define PRE_WIDTH_US 10 // (lower bound for) pre-pulse width in microseconds

IntervalTimer periodTimer;
OneShotTimer t1; // pre-pulse timer  // Note, must avoid PWM clashes: https://github.com/luni64/TeensyTimerTool/wiki/Avoid-PWM-timer-clashes#teensy-41
OneShotTimer t2; // main pulse timer  // Note, must avoid PWM clashes: https://github.com/luni64/TeensyTimerTool/wiki/Avoid-PWM-timer-clashes#teensy-41
OneShotTimer tref; // internal trigger reference timer  // Note, must avoid PWM clashes: https://github.com/luni64/TeensyTimerTool/wiki/Avoid-PWM-timer-clashes#teensy-41

bool await_update = false; // Boolean: 1 if there are new updates to the device settings waiting to be implemented, else 0
uint64_t trigcnt = 0; // Trigger ID upcounter; increments with each external trigger rising edge, whether or not an image is acquired/transmitted

 // preload values
bool out_enabled_pr = true; // Boolean: 1 if LilLaser's optical flashes are enabled.
uint32_t prepulse_us_pr = 300; // Pre-pulse pulse duration, in microseconds
uint32_t mainpulse_us_pr = 100; // Main pulse pulse duration, in microseconds
double prepulse_power_pr = 0.1; // Pre-pulse power (W), on a scale from zero to one
double mainpulse_power_pr = 0.9; // Main pulse power (W), on a scale from zero to one
uint32_t refreprate_hz_pr = 20; // Start out at 20 Hz repetition rate for reference pulses

// Load in active values from preload values
bool out_enabled = out_enabled_pr;
uint32_t prepulse_us = prepulse_us_pr;
uint32_t mainpulse_us = mainpulse_us_pr;
double prepulse_power = prepulse_power_pr;
double mainpulse_power = mainpulse_power_pr;
uint32_t refreprate_hz = refreprate_hz_pr;
uint32_t refperiod_us = (1.0 / refreprate_hz) * 1000000; // convert frequency (Hz) to time (microseconds);

// Instantiate a Chrono object for the led heartbeat and LED trigger display
Chrono heartbeatChrono; 

// external trig callback interrupt service routine
void ISR_exttrig() {
  trigcnt++;
  if ((!await_update) && out_enabled) {
      // Start the pre-pulse laser output
      analogWrite(LASER_PIN, round(prepulse_power * 255));
      t1.trigger(prepulse_us);
  }
}

// The reference-trigger period callback sets the internal reference trigger to high
void ref_trig_callback() {
  if (!await_update) {
    digitalWriteFast(REF_PIN, HIGH);
    tref.trigger(10);
  }
}

// The reference-trigger timer callback only exists to pull the internal reference trigger back to low
void tref_callback()
{
  digitalWriteFast(REF_PIN, LOW);    
}

// The pre-pulse and main pulse timer callbacks are each called exactly once per pulse fire
void t1_callback()
{
  // Pre-pulse timer expired, start the main pulse laser output
  analogWrite(LASER_PIN, round(mainpulse_power * 255));
  t2.trigger(mainpulse_us);
}

void t2_callback()
{
  // Main pulse timer expired, stop all laser output
  analogWrite(LASER_PIN, 0);
}

void setup() {
  // initialize the heartbeat LED and updating LED as output
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);
  pinMode(REF_PIN, OUTPUT);

  // set laser as an output
  analogWriteFrequency(LASER_PIN, 2000000); // smooths out the laser profile sufficiently to avoid seeing the PWM artifacts
  analogWrite(LASER_PIN, 0); // make sure we start things off with no laser output


  // Setup the output timers
  t1.begin(t1_callback);
  t2.begin(t2_callback);
  tref.begin(tref_callback);

  periodTimer.priority(255); // set the long timescale timer interrupt to low priority, so it doesn't interfere with the higher-precision timing
  periodTimer.begin(ref_trig_callback, refperiod_us);

  // attach the external interrupt
  pinMode(EXTTRIG_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(EXTTRIG_PIN), ISR_exttrig, RISING);

  // Initialize SCPI interface
  SCPI_Arduino_Setup(); // note, begins Serial if communication style is Serial
}

void loop() {
  // Blink the heartbeat LED
  if (heartbeatChrono.hasPassed(500)) { // check if the 500 milliseconds of heartbeat have elapsed
    digitalToggleFast(HEARTBEAT_LED_PIN); // write LED state to pin
    heartbeatChrono.restart();
    Serial.println(round(mainpulse_power * 255));
  }
    
  if (await_update) {
      periodTimer.end();
      tref.stop();
      t1.stop();
      t2.stop();

      delayMicroseconds(100); // short superstitious delay to clear things out

      // Bring all outputs back to low, in case they were stuck high when we cut them off
      digitalWriteFast(REF_PIN, LOW);
      digitalWriteFast(LASER_PIN, LOW);

      // Load in active values from preload values
      out_enabled = out_enabled_pr;
      prepulse_us = prepulse_us_pr;
      mainpulse_us = mainpulse_us_pr;
      prepulse_power = prepulse_power_pr;
      mainpulse_power = mainpulse_power_pr;
      refreprate_hz = refreprate_hz_pr;
      refperiod_us = (1.0 / refreprate_hz) * 1000000; // convert frequency (Hz) to time (microseconds)
           
      await_update = false;
      periodTimer.begin(ref_trig_callback, refperiod_us);
  }

  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}

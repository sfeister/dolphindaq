/* LittleLaser
    Flash a $1 pen laser with a (controllable) temporal profile mimicking a Ti:Sapphire high-intensity laser

    With the arrival of each new external / internal trigger pulse:
      A global counter is incremented
      
    Built for a Teensy 4.1 board.
    Based on testing with a Teensy 4.1 board.

    Tested to work fine at repetition rates up to or even slightly beyond 1 kHz.

    PINS:
      23    |   Input, external trigger
      22     |   Output, Pen laser control (this PWM output goes to the base of a transistor which switches the actual laser)
      built-in LED     |   Output, green status LED, heartbeat. Flashes once per second reliably.
            
    Written using the other existing DolphinDAQ projects (PulseGenerator, BlinkSync, and Rosie) as computer code templates.

    Followed TeensyTimerTool tutorial at: https://forum.pjrc.com/threads/59112-TeensyTimerTool
    
    Written by Scott Feister on September 23, 2023. (and continued improvement through 2024)
*/

#include <Arduino.h>
#include <Chrono.h> //non-interrupt, low-precision timing
#include "scpi-def.h"
#include "TeensyTimerTool.h"
using namespace TeensyTimerTool;

#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN LED_BUILTIN
#define LASER_PIN 22 // Note, must avoid PWM clashes: https://github.com/luni64/TeensyTimerTool/wiki/Avoid-PWM-timer-clashes#teensy-41
#define DAC_NT 100 // Number of steps in the DAC temporal profile array

extern const int settings_dac_nt = DAC_NT;
uint32_t settings_dac_dt; // Number of microseconds between updates to the DAC
uint32_t settings_dac_dt_pr; // Preload
double settings_pwm_hz; // Frequency (Hz) of the output laser PWM signal output (analogWrite)
double settings_pwm_hz_pr; // Preload

bool await_update = false; // Boolean: 1 if there are new updates to the device settings waiting to be implemented, else 0
volatile uint64_t trigcnt = 0; // Trigger ID upcounter; increments with each external trigger rising edge, whether or not an image is acquired/transmitted
volatile int32_t dac_index = 0; // Incrementing index specifying which element of the dac_powers array we are currently on
// declared "volatile" for compiler as this value is updated within interrupts

// Powers (0 to 255) that form the laser temporal profile
uint8_t dac_powers[settings_dac_nt];
uint8_t dac_powers_pr[settings_dac_nt]; // preload

// Boolean: 1 if LilLaser's optical flashes are enabled.
bool out_enabled;
bool out_enabled_pr; // preload

// Chrono object for the led heartbeat and LED trigger display
Chrono heartbeatChrono; 

// Interrupt-based periodic timer for timing the digital-to-analog conversion (not a real DAC, but a PWM-based DAC)
PeriodicTimer dacTimer;

// external trig callback interrupt service routine
void ISR_exttrig() {
  trigcnt++;
  if ((!await_update) && out_enabled) {
      // Start the laser output
      dac_index = 0;
      dacTimer.start();
  }
}

// repeatedly called until all elements of dac_powers array have been written
void dac_callback()
{
  if (dac_index < settings_dac_nt) {
    analogWrite(LASER_PIN, dac_powers[dac_index]);
    //Serial.println(dac_powers[dac_index]);
    dac_index++;
  } else {
    analogWrite(LASER_PIN, 0);
    dacTimer.stop(); // end of array; stop the DAC
    //Serial.println("STOPPED.");
    // digitalWriteFast(LASER_PIN, LOW); // zero the laser output
  }
}

void setup() {
  // set up preload values
  out_enabled_pr = true;
  settings_dac_dt_pr = 5;
  settings_pwm_hz_pr = 4.0 * 585937.5; // ideal frequency for 8-bit PWM is 585937.5, any higher will reduce bit range as stated at https://www.pjrc.com/teensy/td_pulse.html

  for (int i = 0; i < settings_dac_nt; i++) {
    dac_powers_pr[i] = i; // a recognizable initialization of DAC powers to an incrementing value
  }

  // Load in active values from preload values
  out_enabled = out_enabled_pr;
  settings_dac_dt = settings_dac_dt_pr;
  settings_pwm_hz = settings_pwm_hz_pr;

  for (int i = 0; i < settings_dac_nt; i++) {
    dac_powers[i] = dac_powers_pr[i];
  }

  // initialize the heartbeat LED and updating LED as output
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);

  // set laser as an output
  analogWriteFrequency(LASER_PIN, settings_pwm_hz); // ideal frequency for 8-bit PWM is 585937.5, any higher will reduce bit range
  analogWrite(LASER_PIN, 0); // make sure we start things off with no laser output

  // Setup the output timers
  dacTimer.begin(dac_callback, settings_dac_dt, false);
  // TODO: laser output is time-sensitive! high priority, higher than communication priority. 

  // attach the external interrupt
  pinMode(EXTTRIG_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(EXTTRIG_PIN), ISR_exttrig, RISING);
  NVIC_SET_PRIORITY(IRQ_GPIO6789, 0); // set all external interrupts to maximum priority level
  // Note that all interrupts using IRQ_GPIO6789 according to https://forum.pjrc.com/index.php?threads/teensy-4-set-interrupt-priority-on-given-pins.59828/post-231312

  // Initialize SCPI interface
  SCPI_Arduino_Setup(); // note, begins Serial if communication style is Serial
}

void loop() {
  // Blink the heartbeat LED
  if (heartbeatChrono.hasPassed(500)) { // check if the 500 milliseconds of heartbeat have elapsed
    digitalToggleFast(HEARTBEAT_LED_PIN); // write LED state to pin
    heartbeatChrono.restart();
  }
    
  if (await_update) {
      // stop the laser output
      dacTimer.stop();
      digitalWriteFast(LASER_PIN, LOW);

      // Load in active values from preload values
      out_enabled = out_enabled_pr;
      settings_dac_dt = settings_dac_dt_pr;
      settings_pwm_hz = settings_pwm_hz_pr;

      for (int i = 0; i < settings_dac_nt; i++) {
        dac_powers[i] = dac_powers_pr[i];
      }

      // Update settings themselves on device
      analogWriteFrequency(LASER_PIN, settings_pwm_hz);
      dacTimer.setPeriod(settings_dac_dt);
           
      await_update = false;
  }

  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}

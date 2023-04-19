/* PulseGenerator
    Device to send delayed outputs.

    With the arrival of each new external / internal trigger pulse:
      A global counter is incremented
      
    Built for a Teensy 4.1 board or Teensy 3.2 board.
    Based on testing with a Teensy 3.2 board, expect in the range 20-microsecond precision. Jitter is less than 5 microseconds.

    Tested to work fine at repetition rates up to or even slightly beyond 1 kHz.

    PINS:
      23    |   Input, external trigger (optional; internally triggered by default)
      9     |   Output, Channel 1
      10    |   Output, Channel 2
      11    |   Output, Channel 3
      12    |   Output, Channel 4
      3     |   Output, green status LED, heartbeat. Flashes once per second reliably.
      4     |   Output, blue LED indicating that a trigger has arrived.
      19    |   Output, Reference. HIGH at time zero, LOW when all pulses have completed firing

            
    Followed TeensyTimerTool tutorial at: https://forum.pjrc.com/threads/59112-TeensyTimerTool
    
    Works for delays up to about 10 milliseconds.

    Written by Scott Feister on August 4, 2023.
*/

#include <Arduino.h>
#include <Chrono.h> //non-interrupt, low-precision timing
#include "scpi-def.h"
#include "TeensyTimerTool.h"
using namespace TeensyTimerTool;

#define REF_PIN 19 // reference / internal trigger pin
#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN 3
#define TRIGGERED_LED_PIN 4 // Pin that is active for a brief time after each trigger is being output
#define CH1_PIN 9
#define CH2_PIN 10
#define CH3_PIN 11
#define CH4_PIN 12
#define WIDTH_US 10 // (lower bound for) pulse width in microseconds

IntervalTimer periodTimer;
OneShotTimer tref;
OneShotTimer t1;
OneShotTimer t2;
OneShotTimer t3;
OneShotTimer t4;

uint64_t trigcnt = 0; // global, incrementing trigger count
bool await_update = false; // whether to pause to update the frequency, etc
uint8_t trigLedState = LOW;

uint8_t trig_mode = 0; // 0 for internal triggering, 1 for external triggering
bool out_enabled = true; // whether outputs are enabled

uint32_t ch1_delay_us_pr = 100; // preload values
uint32_t ch2_delay_us_pr = 100;
uint32_t ch3_delay_us_pr = 100;
uint32_t ch4_delay_us_pr = 100;

// Load in active values from preload values
uint32_t ch1_delay_us = ch1_delay_us_pr; // default delay
uint32_t ch2_delay_us = ch2_delay_us_pr;
uint32_t ch3_delay_us = ch3_delay_us_pr;
uint32_t ch4_delay_us = ch3_delay_us_pr;

double reprate_hz = 10;  // Start out at 10 Hz repetition rate for pulses
double period_us = (1.0 / reprate_hz) * 1000000; // convert frequency (Hz) to time (microseconds)

// Instantiate a Chrono object for the led heartbeat and LED trigger display
Chrono heartbeatChrono; 
Chrono trigLEDChrono; 

void internal_trig_callback() {
  if ((digitalReadFast(REF_PIN) == LOW) && (!await_update) && (trig_mode == 0) && out_enabled) {
    trigcnt++;
    fire_pulses();    
  } /* else {
    Serial.print("REF_PIN: ");
    Serial.print(digitalReadFast(REF_PIN));
    Serial.print(", await_update: ");
    Serial.print(await_update);
    Serial.print(", trig_mode: ");
    Serial.print(trig_mode);
    Serial.print(", out_enabled: ");
    Serial.println(out_enabled);
  } */ //DEBUG
}

// external trig callback interrupt service routine
void ISR_exttrig() {
  if ((digitalReadFast(REF_PIN) == LOW) && (!await_update) && (trig_mode == 1) && out_enabled) {
    trigcnt++;
    fire_pulses();
  }
}

// The fire_pulses() function should be called to initiate one set of pulses, for example, called once per external trigger
// It is called when we set the reference trigger high and then schedule all the other pulses according to their delays
// It should execute essentially at time zero
void fire_pulses() {
  digitalWriteFast(REF_PIN, HIGH);    
  t1.trigger(ch1_delay_us);
  t2.trigger(ch2_delay_us);
  t3.trigger(ch3_delay_us);
  t4.trigger(ch4_delay_us);
  tref.trigger(max(ch1_delay_us, max(ch2_delay_us, max(ch3_delay_us, ch4_delay_us))) + WIDTH_US + 100);
  digitalWriteFast(TRIGGERED_LED_PIN, HIGH);    
  trigLEDChrono.restart();
}


// The trigger-reference callback only exists to pull the reference trigger back to low
void tref_callback()
{
  digitalWriteFast(REF_PIN, LOW);    
}

// The four channel callbacks are called both when the channel should start, and when it should and stop, its pulse
// These channel callbacks both activate and then lower the voltage of their channel
void ch1_callback()
{
    if (digitalReadFast(CH1_PIN) == LOW) {
      digitalWriteFast(CH1_PIN, HIGH);    
      t1.trigger(WIDTH_US); // trigger the timer again in WIDTH_US microseconds
    } else {
      digitalWriteFast(CH1_PIN, LOW);    
    }
}


void ch2_callback()
{
    if (digitalReadFast(CH2_PIN) == LOW) {
      digitalWriteFast(CH2_PIN, HIGH);    
      t2.trigger(WIDTH_US); // trigger the timer again in WIDTH_US microseconds
    } else {
      digitalWriteFast(CH2_PIN, LOW);    
    }
}

void ch3_callback()
{
    if (digitalReadFast(CH3_PIN) == LOW) {
      digitalWriteFast(CH3_PIN, HIGH);    
      t3.trigger(WIDTH_US); // trigger the timer again in WIDTH_US microseconds
    } else {
      digitalWriteFast(CH3_PIN, LOW);    
    }
}

void ch4_callback()
{
    if (digitalReadFast(CH4_PIN) == LOW) {
      digitalWriteFast(CH4_PIN, HIGH);    
      t4.trigger(WIDTH_US); // trigger the timer again in WIDTH_US microseconds
    } else {
      digitalWriteFast(CH4_PIN, LOW);    
    }
}

void setup() {
  // initialize the heartbeat LED and updating LED as output
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);
  pinMode(TRIGGERED_LED_PIN, OUTPUT);
  pinMode(REF_PIN, OUTPUT);

  // set primary and secondary channel output pins
  pinMode(CH1_PIN, OUTPUT);
  pinMode(CH2_PIN, OUTPUT);
  pinMode(CH3_PIN, OUTPUT);
  pinMode(CH4_PIN, OUTPUT);

  // Setup the output timers
  t1.begin(ch1_callback);
  t2.begin(ch2_callback);
  t3.begin(ch3_callback);
  t4.begin(ch4_callback);
  tref.begin(tref_callback);

  periodTimer.priority(255); // set the long timescale timer interrupt to low priority, so it doesn't interfere with the higher-precision timing
  periodTimer.begin(internal_trig_callback, period_us);

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
  }
  
  if (trigLEDChrono.hasPassed(5, true)) {
    trigLEDChrono.stop();
    digitalWriteFast(TRIGGERED_LED_PIN, LOW);
  }
  
  if (await_update) {
      periodTimer.end();
      tref.stop();
      t1.stop();
      t2.stop();
      t3.stop();
      t4.stop();

      delayMicroseconds(100); // short superstitious delay to clear things out

      // Bring all triggered outputs back to low, in case they were stuck high when we cut them off
      digitalWriteFast(REF_PIN, LOW);
      digitalWriteFast(CH1_PIN, LOW);
      digitalWriteFast(CH2_PIN, LOW);
      digitalWriteFast(CH3_PIN, LOW);
      digitalWriteFast(CH4_PIN, LOW);

      ch1_delay_us = ch1_delay_us_pr; // Update with preload values
      ch2_delay_us = ch2_delay_us_pr;
      ch3_delay_us = ch3_delay_us_pr;
      ch4_delay_us = ch4_delay_us_pr;
      
      double period_us = (1.0 / reprate_hz) * 1000000; // convert frequency (Hz) to time (microseconds)

      /* 
      Serial.printf("t1  %8.2f milliseconds\n",t1.getMaxPeriod()*1000);
      Serial.printf("t2  %8.2f milliseconds\n",t2.getMaxPeriod()*1000);
      Serial.printf("t3  %8.2f milliseconds\n",t3.getMaxPeriod()*1000);
      Serial.printf("t4  %8.2f milliseconds\n",t4.getMaxPeriod()*1000);
      Serial.printf("tref  %8.2f milliseconds\n",tref.getMaxPeriod()*1000);
      */ // DEBUG
      
      await_update = false;
      periodTimer.begin(internal_trig_callback, period_us);
  }
  
  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}

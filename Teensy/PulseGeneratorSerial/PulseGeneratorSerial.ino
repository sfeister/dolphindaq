/* PulseGenerator
    Device to send delayed outputs.

    With the arrival of each new external / internal trigger pulse:
      A global counter is incremented
      
    Built for a Teensy 4.1 board

    PINS:
      23    |   Input, external trigger (optional)
      3     |   Output, green status LED, heartbeat. Flashes once per second reliably.
      4     |   Output, blue LED indicating that a trigger has arrived.
      19    |   Output, Time zero
      10    |   Output, Channel 0
      11    |   Output, Channel 1
      12    |   Output, Channel 2
            
    Followed TeensyTimerTool tutorial at: https://forum.pjrc.com/threads/59112-TeensyTimerTool
    
    Written by Scott Feister on August 4, 2023.
*/

#include <Arduino.h>
#include <Metro.h> //non-interrupt, low-precision timing
#include "scpi-def.h"
#include "TeensyTimerTool.h"
using namespace TeensyTimerTool;

#define REF_PIN 19 // reference / internal trigger pin
#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN 3
#define TRIGGERED_LED_PIN 4 // Pin that is active for a brief time after each trigger is being output
#define CH1_PIN 10
#define CH2_PIN 11
#define CH3_PIN 12
#define WIDTH_US 10 // (lower bound for) pulse width in microseconds

IntervalTimer periodTimer;
OneShotTimer tref; 
OneShotTimer t1; 
OneShotTimer t2; 
OneShotTimer t3; 

uint64_t trigcnt = 0; // global, incrementing trigger count
bool await_update = false; // whether to pause to update the frequency, etc
uint8_t heartbeatState = LOW;
uint8_t trigLedState = LOW;

uint32_t ch1_delay_us_pr = 100; // preload values
uint32_t ch2_delay_us_pr = 100;
uint32_t ch3_delay_us_pr = 100;

uint32_t ch1_delay_us = ch1_delay_us_pr; // default delay
uint32_t ch2_delay_us = ch2_delay_us_pr;
uint32_t ch3_delay_us = ch3_delay_us_pr;

double reprate_hz = 20;  // Start out at 20 Hz period
double period_us = (1.0 / reprate_hz) * 1000000; // convert frequency (Hz) to time (microseconds)

// Instanciate a metro object for the led heartbeat, and set the interval to 500 milliseconds
Metro heartbeatMetro = Metro(500);

void ISR_exttrig() {
  trigcnt++;
}

void time_zero_callback() {
  // executes time zero
  digitalWriteFast(REF_PIN, HIGH);    
  t1.trigger(ch1_delay_us);
  t2.trigger(ch2_delay_us);
  t3.trigger(ch3_delay_us);
  tref.trigger(WIDTH_US);
}

void tref_callback()
{
  digitalWriteFast(REF_PIN, LOW);    
}

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
      t1.trigger(WIDTH_US); // trigger the timer again in WIDTH_US microseconds
    } else {
      digitalWriteFast(CH3_PIN, LOW);    
    }
}

void setup() {
  // initialize the heartbeat LED and updating LED as output
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);
  pinMode(TRIGGERED_LED_PIN, OUTPUT);

  // set primary and secondary channel output pins
  pinMode(CH1_PIN, OUTPUT);
  pinMode(CH2_PIN, OUTPUT);
  pinMode(CH3_PIN, OUTPUT);

  // Setup the channel timers
  t1.begin(ch1_callback);
  t2.begin(ch2_callback);
  t3.begin(ch3_callback);
  tref.begin(tref_callback);

  periodTimer.priority(255);
  periodTimer.begin(time_zero_callback, period_us);

  // attach the external interrupt
  pinMode(EXTTRIG_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(EXTTRIG_PIN), ISR_exttrig, RISING);

  // Initialize SCPI interface
  SCPI_Arduino_Setup(); // note, begins Serial if communication style is Serial
}

void loop() {
  // Blink the heartbeat LED
  if (heartbeatMetro.check() == 1) { // check if the metro has passed its interval
    heartbeatState = ~heartbeatState; // toggle LED state
    digitalWriteFast(HEARTBEAT_LED_PIN, heartbeatState); // write LED state to pin
  }

  if (await_update) {
      await_update = false;
      periodTimer.end();
      tref.stop();
      t1.stop();
      t2.stop();
      t3.stop();

      delayMicroseconds(100); // short superstitious delay to clear things out
      
      ch1_delay_us = ch1_delay_us_pr; // Update with preload values
      ch2_delay_us = ch2_delay_us_pr;
      ch3_delay_us = ch3_delay_us_pr;

      double period_us = (1.0 / reprate_hz) * 1000000; // convert frequency (Hz) to time (microseconds)
      periodTimer.begin(time_zero_callback, period_us);
  }
  
  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}

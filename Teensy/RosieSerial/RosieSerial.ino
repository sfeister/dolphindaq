/* Rosie
    Decimates incoming triggers.

    Built for a Teensy 4.1 board

    PINS:
      23    |   Input, external trigger
      3     |   Output, green status LED, heartbeat. Flashes once per second reliably.
      4     |   Output, blue LED indicating that a trigger has arrived.
      19    |   Output, Reference. HIGH at time zero, LOW when all pulses have completed firing
      9     |   Output, Channel 1
      10    |   Output, Channel 2
      11    |   Output, Channel 3
      12    |   Output, Channel 4
            
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
#define TRIGOUT_PIN 9
#define WIDTH_US 10 // (lower bound for) pulse width in microseconds

OneShotTimer t1;

bool await_update = false; // Boolean: 1 if there are new updates to the device settings waiting to be implemented, else 0
bool out_enabled = true; // Boolean: 1 if Rosie's decimated trigger outputs are enabled.
bool out_enabled_preload = true; // Boolean: 1 if Rosie's decimated trigger outputs are enabled. (Preload)
uint64_t trigcnt = 0; // Trigger ID upcounter; increments with each external trigger rising edge, whether or not an image is acquired/transmitted
uint32_t decimate = 100; // The ratio of external triggers to acquisition triggers. If decimate = 1, no decimation. If decimate = 2, every other trigger results in image acquisition. If decimate = 10, every tenth trigger results in image acquisition. Etc.
uint64_t out_first = 0; // The first trigger number, of the most recent output enabled
uint64_t out_count = 0; // The count of triggers output since enabling outputs / restarting DAQ

uint8_t heartbeatState = LOW;
uint8_t trigLedState = LOW;


// Instantiate a metro object for the led heartbeat, and set the interval to 500 milliseconds
Metro heartbeatMetro = Metro(500);

// external trig callback interrupt service routine
void ISR_exttrig() {
    // (1) Increment trigger counter
    trigcnt++;

    // (2) Activate the DAQ and trigger output pipeline (unless decimation prevents this)
    if (!(trigcnt % decimate)) // proceed only for divisible numbers
    {
      if (!await_update && out_enabled) {
        digitalWriteFast(TRIGOUT_PIN, HIGH); 
        t1.trigger(WIDTH_US);
        if (out_first < 1) { // checks for unset out_first (initial trigger), and sets it to current value
          out_first = trigcnt; // starting trigger since enabling
        }
        out_count++;
      }
    }
}

// Disable ROSIE trigger output
void output_disable() {
  out_enabled = false;
}

// Enable ROSIE trigger output
void output_enable() {
  out_first = 0; // reset the output first-trigger identification, since we're restarting the output triggers here
  out_count = 0; // reset the output trigger count
  out_enabled = true;
}

// The trigger-reference callback only exists to pull the trigger output back to low
void t1_callback()
{
  digitalWriteFast(TRIGOUT_PIN, LOW);    
}

void setup() {
  // initialize the heartbeat LED and updating LED as output
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);
  pinMode(TRIGGERED_LED_PIN, OUTPUT);

  // set trigger output pin
  pinMode(TRIGOUT_PIN, OUTPUT);

  // Setup the output timer
  t1.begin(t1_callback);

  // attach the external interrupt
  pinMode(EXTTRIG_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(EXTTRIG_PIN), ISR_exttrig, RISING);

  // Initialize SCPI interface
  SCPI_Arduino_Setup(); // note, begins Serial if communication style is Serial
}

void loop() {
  // Blink the heartbeat LED
  if (heartbeatMetro.check() == 1) { // check if the metro has passed its interval
    heartbeatState = !heartbeatState; // toggle LED state
    digitalWriteFast(HEARTBEAT_LED_PIN, heartbeatState); // write LED state to pin
  }

  // Check for settings updates
  if (await_update) {
    delayMicroseconds(100); // short delay to clear out any existing triggered pulses
    if (out_enabled_preload != out_enabled){ // Inhibit or facilitate output
      if (out_enabled_preload) {
        output_enable();
      }
      else {
        output_disable();
      }
    }
    await_update = false;
  }
  
  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}

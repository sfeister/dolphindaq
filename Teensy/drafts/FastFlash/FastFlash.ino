/* FastFlash
    With each trigger received, flashes an LED very quickly (using Digital  I/O).

    Built for a Teensy 4.1 board

    PINS:
      23    |   Input, external trigger. Connect this pin to the trigger source.
      5     |   Output, fast flashing LED. Connect from this pin to a 220 ohm resistor thru the LED and then to ground.
      3     |   Output, green status LED, heartbeat. Flashes once per second reliably. Connect from this pin to a 220 ohm resistor thru the LED and then to ground.
      4     |   Output, Debug. HIGH when an acquisition starts, LOW when the acquisition stops. No need to connect this pin at all.
            
    Written by Scott Feister on December 7, 2023.
*/

#include <Arduino.h>
#include <Chrono.h> //non-interrupt, low-precision timing
#include "scpi-def.h"

#define DEBUG_PIN 4 // just for debugging
#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN 3
#define FAST_LED_PIN 5

bool await_update = false; // Boolean: 1 if there are new updates to the device settings waiting to be implemented, else 0
uint64_t trigcnt = 0; // Trigger ID upcounter; increments with each external trigger rising edge, whether or not an image is acquired/transmitted

// Instantiate a Chrono object for the led heartbeat and LED trigger display
Chrono heartbeatChrono; 

// external trig callback interrupt service routine
void ISR_exttrig() {
    // (1) Increment trigger counter
    trigcnt++;

    // (2) FLASH LED AS FAST AS POSSIBLE
    digitalWriteFast(FAST_LED_PIN, HIGH);
    digitalWriteFast(FAST_LED_PIN, LOW);
}

void setup() {
  pinMode(DEBUG_PIN, OUTPUT);

  // initialize the heartbeat LED and updating LED as output
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);

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

  // Check for settings updates
  if (await_update) {
    delayMicroseconds(100); // short delay to clear out any existing triggered pulses
    // TODO: Implement all updates
    await_update = false;
  }
  
  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}
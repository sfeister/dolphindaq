/* Counter
    Simple pulse counter with SCPI interface. Prints the pulse count every 50 ms.

    Built for a Teensy 4.1 board or Teensy 4.0 board.

    PINS:
      23    |   Input, external trigger (the pulses to count)
      3     |   Output, green status LED, heartbeat. Flashes once per second reliably.

    TODO:
      Raise the priority of the ISR interrupt above all other interrupts.
          
    Written by Scott Feister on March 25, 2024.
*/

#include <Arduino.h>
#include <Chrono.h> //non-interrupt, low-precision timing
#include "scpi-def.h"

#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN 3

uint64_t trigcnt = 0; // Trigger ID upcounter; increments with each external trigger rising edge

// Instantiate a Chrono object for the led heartbeat and for the printing of the count to serial port
Chrono heartbeatChrono; 
Chrono countPrintChrono; 

// external trig callback interrupt service routine
void ISR_exttrig() {
    // Increment trigger counter
    trigcnt++;
}

void setup() {
  // initialize the heartbeat LED as output
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

  if (countPrintChrono.hasPassed(50)) { // check if the 50 milliseconds of the count print chrono have elapsed
    Serial.print("COUNT: ")
    Serial.println(trigcnt)
  }

  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}

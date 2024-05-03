/* Counter
    Simple pulse counter with SCPI interface. Prints the pulse count every 100 ms.

    Built for a Teensy 4.1 board or Teensy 4.0 board.

    PINS:
      23    |   Input, external trigger (the pulses to count)
      built-in LED     |   Output, green status LED, heartbeat. Flashes once per second reliably.
          
    Written by Scott Feister on March 25, 2024.
*/

#include <Arduino.h>
#include <Chrono.h> //non-interrupt, low-precision timing
#include "scpi-def.h"

#define PHOTOTRANSISTOR_PIN A0
#define ELECTRON_PIN 9
#define PROTON_PIN 10

uint32_t analog_write_hz = 10000;  // deliberately send out fairly rough PWM signals
uint16_t electron_power_debug = 255;
uint16_t proton_power_debug = 255;
uint16_t laser_raw;
double laser_power, laser_power_tmp, electron_current, proton_current;
int proton_pin = PROTON_PIN;
int electron_pin = ELECTRON_PIN;
uint16_t system_mode = 0; // 0 for regular mode, 1 for debug mode

void setup() {
  analogWriteFrequency(PROTON_PIN, analog_write_hz);
  analogWriteFrequency(ELECTRON_PIN, analog_write_hz);
  // Initialize SCPI interface
  SCPI_Arduino_Setup(); // note, begins Serial if communication style is Serial
}

void loop() {
  if (system_mode > 0) { // DEBUG mode
    analogWrite(ELECTRON_PIN, electron_power_debug);
    analogWrite(PROTON_PIN, proton_power_debug);
    Serial.print("Raw ADC (rapid sequence): ");
    for (int i = 0; i < 10; i++) {
      laser_raw = analogRead(PHOTOTRANSISTOR_PIN);
      Serial.print(laser_raw);
      Serial.print(", ");
    }
    Serial.println(laser_raw);
    delay(1); // delay for stability (don't want to push the SCPI too hard!)
  } else { // REGULAR mode
    // Read in the laser power
    laser_raw = analogRead(PHOTOTRANSISTOR_PIN);
    laser_power_tmp = laser_raw / 1023.0; // the power of the laser at this moment, on a scale of 0 to 1; assumes 10-bit analogRead, the default for Teensy 4.0 and Arduino
    laser_power = -(laser_power_tmp - 1.0); // flip polarity for Sidekick Model 4

    // Compute electron and proton current based on the current laser power (TODO: based also on the running average of laser power)
    electron_current = laser_power; // emitted electron beam current at this moment, on a scale of 0 to 1
    proton_current = laser_power*laser_power; // emitted proton beam current at this moment, on a scale of 0 to 1

    // Encode the electron and proton currents as PWM values of their respective
    analogWrite(ELECTRON_PIN, round(electron_current * 255));
    analogWrite(PROTON_PIN, round(proton_current * 255));
  }

  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}

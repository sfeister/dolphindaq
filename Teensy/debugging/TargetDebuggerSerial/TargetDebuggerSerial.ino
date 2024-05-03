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
#include <ADC.h> // from https://github.com/pedvide/ADC

#include "scpi-def.h"

#define PHOTOTRANSISTOR_PIN A0
#define ELECTRON_PIN 9
#define PROTON_PIN 10
#define MILLIS_AVG 2 // milliseconds for which to hang software and average the ADC value

uint32_t analog_write_hz = 10000;  // deliberately send out fairly rough PWM signals
uint16_t electron_power_debug = 255;
uint16_t proton_power_debug = 255;
uint16_t laser_raw;
double laser_power, laser_power_tmp, electron_current, proton_current;
int proton_pin = PROTON_PIN;
int electron_pin = ELECTRON_PIN;
uint16_t system_mode = 0; // 0 for regular mode, 1 for debug mode

Chrono avgChrono; 

// ADC SETUP FROM EXAMPLE
const int readPin_adc_0 = PHOTOTRANSISTOR_PIN;
ADC *adc = new ADC(); // adc object
const uint32_t initial_average_value = 2048;
// END ADC SETUP FROM EXAMPLE

void setup() {
  analogWriteFrequency(PROTON_PIN, analog_write_hz);
  analogWriteFrequency(ELECTRON_PIN, analog_write_hz);



    // ADC SETUP FROM EXAMPLE
  pinMode(readPin_adc_0, INPUT_DISABLE); // Not sure this does anything for us
  adc->adc0->setAveraging(32);   // set number of averages // bumped up here for dealing with PWM input
  adc->adc0->setResolution(10); // set bits of resolution
  adc->adc0->setSamplingSpeed(ADC_settings::ADC_SAMPLING_SPEED::VERY_HIGH_SPEED); // set high sampling speed to get a better average
  // end ADC SETUP FROM EXAMPLE

  // Initialize SCPI interface
  SCPI_Arduino_Setup(); // note, begins Serial if communication style is Serial
}

void loop() {
  if (system_mode > 0) { // DEBUG mode
    analogWrite(ELECTRON_PIN, electron_power_debug);
    analogWrite(PROTON_PIN, proton_power_debug);
    Serial.print("Raw ADC (rapid sequence): ");
    for (int i = 0; i < 10; i++) {
      laser_raw = adc->adc0->analogRead(PHOTOTRANSISTOR_PIN);
      Serial.print(laser_raw);
      Serial.print(", ");
    }
    Serial.println(laser_raw);
    delay(1); // delay for stability (don't want to push the SCPI too hard!)
  } else { // REGULAR mode
    // Read in the laser power for 1 millisecond
    uint64_t laser_raw_sum = 0;
    
    avgChrono.restart();
    int counter = 0;
    do {
      laser_raw_sum += adc->adc0->analogRead(PHOTOTRANSISTOR_PIN);
      counter++;
    }
    while (!avgChrono.hasPassed(MILLIS_AVG));

    double laser_raw_avg = laser_raw_sum / double(counter);
    laser_raw = round(laser_raw_avg);
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

/* TargetFoil
    Crude mimickry of a thin foil being struck with a laser and emitting a flash of protons and electrons.

    Built for a Teensy 4.0 board.
    Based on testing with a Teensy 4.0 board.

    Tested to work fine at repetition rates up to or even slightly beyond 1 kHz.

    PINS:
      14/A0    |   Input, phototransistor (incoming laser pulse)
      9        |   Output, green "electrons" LED (flashes according to incoming laser pulse)
      10       |   Output, blue "protons" LED (flashes according to incoming laser pulse)
            
    Part of DolphinDAQ. No serial communication needed here, at least for now. It's more of a standalone device.
    
    Written by Scott Feister on September 25, 2023.

    MODIFIED FOR SIDEKICK MODEL 4 to reverse phototransistor polarization expectations. E.g. 1023 becomes 0, and 0 becomes 1023.
*/

#include <Arduino.h>
#include <Chrono.h> //non-interrupt, low-precision timing
#include <ADC.h> // from https://github.com/pedvide/ADC

#define PHOTOTRANSISTOR_PIN A0
#define ELECTRON_PIN 9
#define PROTON_PIN 10
#define MILLIS_AVG 2 // milliseconds for which to hang software and average the ADC value

const uint32_t analog_write_hz = 10000;  // deliberately send out fairly rough PWM signals
uint16_t laser_raw;
double laser_power, laser_power_tmp, electron_current, proton_current;
int proton_pin = PROTON_PIN;
int electron_pin = ELECTRON_PIN;

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
}

void loop() {
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

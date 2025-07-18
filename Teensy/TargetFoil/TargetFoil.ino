/* TargetFoil (SIDEKICK MODEL 4 EDITION ONLY FOR LED DRIVER MODULES)
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
    Updated 2024-08-07 for increased PWM rate.
    UPDATED 2024-09-06 FOR SIDEKICK MODEL 4 ONLY to lower analogWrite frequency to 20 kHz for the LED driver module limitation
*/

#include <Arduino.h>

#define PHOTOTRANSISTOR_PIN A0
#define ELECTRON_PIN 9
#define PROTON_PIN 10
#define ANALOG_WRITE_PWM_HZ 20000 // fastest control rate of LED driver module in Sidekick Model 4 is 20 kHz, according the Amazon.com specs

int laser_raw, electron_raw, proton_raw;
double laser_power, electron_current, proton_current;

void setup() {
  analogWriteFrequency(PROTON_PIN, ANALOG_WRITE_PWM_HZ); // try and smooth the PWM of the leds
  analogWriteFrequency(ELECTRON_PIN, ANALOG_WRITE_PWM_HZ); // try and smooth the PWM of the leds
}

void loop() {
  // Read in the laser power
  laser_raw = analogRead(PHOTOTRANSISTOR_PIN);
  laser_power = laser_raw / 1023.0; // the power of the laser at this moment, on a scale of 0 to 1; assumes 10-bit analogRead, the default for Teensy 4.0 and Arduino

  // Compute electron and proton current based on the current laser power (TODO: based also on the running average of laser power)
  electron_current = laser_power; // emitted electron beam current at this moment, on a scale of 0 to 1
  proton_current = laser_power*laser_power; // emitted proton beam current at this moment, on a scale of 0 to 1

  // Encode the electron and proton currents as PWM values of their respective
  analogWrite(ELECTRON_PIN, round(electron_current * 255));
  analogWrite(PROTON_PIN, round(proton_current * 255));
}

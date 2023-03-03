/* BlinkSync
 *  Device to physically flash unique patterns of LEDs, to help with confirming trigger-number synchronization of multiple triggered cameras.
 *  
 *  Thanks to ChatGPT for the great name, 'BlinkSync'! ;-)
 *  
 *  With the arrival of each new external trigger pulse:
 *    A global counter is incremented
 *    LEDs display the 8-bit count in binary, for 200 microseconds
 *    LEDs turn off again
 *    
 *  Purpose is to help with synchronizing cameras. Camera can point at this counter and make sure it sees the same count as another camera who nominally is taking the same picture on the same frame.
 *  Tested with external triggering up to 1 kHz.
 *  
 *  Built for a Teensy 4.1 board
 *  
 *  PINS:
 *    23    |   Input, external trigger
 *    19    |   Output, reference trigger at 20 Hz with 10% duty cycle (optional usage is to bridge pins 19 and 23)
 *    5     |   Output, green status LED, heartbeat. Flashes once per second reliably.
 *    6     |   Output, blue status LED. Indicates "actively updating data." When lit, data should not be read.
 *    12    |   Output, data LED. Pin 12 is the least-significant bit of the eight-bit unsigned integer count.
 *    11    |   Output, data LED.
 *    10    |   Output, data LED.
 *    ...
 *    5     |   Output, data LED. 5 is the most-significant bit of the eight-bit unsigned integer count.
 *    
 *  Followed IntervalTimer tutorial at: https://www.pjrc.com/teensy/td_timing_IntervalTimer.html
 *  
 *  Written by Scott Feister on August 3, 2023.
*/

#define REF_HZ 20
#define REF_PIN 19
#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN 3;
#define UPDATING_LED_PIN 4; // Pin that is active only during the short period when data LEDs are in process of being updated
const int DATA_LED_PINS[8] = {12,11,10,9,8,7,6,5}; // Use pins 5-12 as the one byte of data display

uint64_t trigcnt = 0; // global, incrementing trigger count
uint8_t dataByte = 0; // one byte of data to be written on the data LEDs
bool dataBits[8]; // an array of zeros and ones, the data for this byte

// the setup() method runs once, when the sketch starts

void ISR_exttrig() {
  trigcnt++;
  
  // Compute the data bits to write, based on the current trigger count
  dataByte = trigcnt % 256; // modulus of 2^8
  for (int i=0; i < 8; i++){
    dataBits[i] = (dataByte & ( 1 << i )) >> i; // extract the i-th bit's 0 or 1 value, with help from https://stackoverflow.com/questions/2249731/how-do-i-get-bit-by-bit-data-from-an-integer-value-in-c
  }

  // Write data to all eight data LEDs
  digitalWriteFast(UPDATING_LED_PIN, HIGH);  // Indicate that data LEDs are now being updated
  for (int i=0; i < 8; i++){
    digitalWriteFast(DATA_LED_PINS[i], dataBits[i]);   // set the LED value
  }
  digitalWriteFast(UPDATING_LED_PIN, LOW);  // Indicate that data LED updates are complete

  delayMicroseconds(200); // leave data active on LEDs for 200 microseconds

  // Shut off all eight data LEDs
  digitalWriteFast(UPDATING_LED_PIN, HIGH);  // Indicate that data LEDs are now being updated
  for (int i=0; i < 8; i++){
    digitalWriteFast(DATA_LED_PINS[i], LOW);   // set the LED value
  }
  digitalWriteFast(UPDATING_LED_PIN, LOW);  // Indicate that data LED updates are complete
}


void setup() {
  // Begin a 1 kHz reference signal
  analogWriteFrequency(REF_PIN, REF_HZ); // Set the frequency of the reference pulse
  analogWrite(REF_PIN, 26); // Creates a reference pulse of duty cycle 26/255 = 10%
  
  // initialize the heartbeat LED and updating LED as output
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);
  pinMode(UPDATING_LED_PIN, OUTPUT);

  // initialize the data LEDs as output
  for (int i=0; i < 8; i++){
    pinMode(DATA_LED_PINS[i], OUTPUT);
  }

  // initialize the standard serial interface for debugging
  Serial.begin(9600);

  // attach the external interrupt
  pinMode(EXTTRIG_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(EXTTRIG_PIN), ISR_exttrig, RISING);
}

// the loop() methor runs over and over again,
// as long as the board has power

void loop() {
  // Blink the heartbeat LED once
  digitalWriteFast(HEARTBEAT_LED_PIN, HIGH);   // set the heartbeat LED on
  delay(500);                  // wait for a second
  digitalWriteFast(HEARTBEAT_LED_PIN, LOW);    // set the heartbeat LED off
  delay(500);                  // wait for a second


  Serial.print("Trigger count: ");
  Serial.println(trigcnt);
  
  // Print debug message over Serial
  Serial.print("dataByte: ");
  Serial.println(dataByte);
  Serial.print("dataBits: ");
  for (int i=0; i < 8; i++){
    Serial.print(dataBits[i]);
  }
  Serial.println("");
}

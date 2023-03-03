/* Eight LEDs. Should maintain an array of eight LEDs.
 *  Eventual goal is to display data on these LEDs.
*/

// Teensy 2.0 has the LED on pin 11
// Teensy++ 2.0 has the LED on pin 6
// Teensy 3.x / Teensy LC have the LED on pin 13
const int builtInLedPin = 13; // Pin for the heartbeat signal
const int heartbeatLedPin = 3; // Pin for the heartbeat signal
const int updatingLedPin = 4; // Pin that is active only during the short period when bits are being updated

//const int dataLedPins[8] = {5,6,7,8,9,10,11,12}; // Use pins 5-12 as the one byte of data display
const int dataLedPins[8] = {12,11,10,9,8,7,6,5}; // Use pins 5-12 as the one byte of data display
uint64_t trigcnt = 0; // global, incrementing trigger count
uint8_t dataByte = 0; // one byte of data to be written on the data LEDs
bool dataBits[8]; // an array of zeros and ones, the data for this byte

// the setup() method runs once, when the sketch starts

void setup() {
  // initialize the built-in LED, heartbeat LED, and updating LED as output
  pinMode(builtInLedPin, OUTPUT);
  pinMode(heartbeatLedPin, OUTPUT);
  pinMode(updatingLedPin, OUTPUT);

  // initialize the data LEDs as output
  for (int i=0; i < 8; i++){
    pinMode(dataLedPins[i], OUTPUT);
  }

  // initialize the standard serial interface for debugging
  Serial.begin(9600);
}

// the loop() methor runs over and over again,
// as long as the board has power

void loop() {
  
  Serial.print("Trigger count: ");
  Serial.println(trigcnt);

  // Compute the data bits to write, based on the current trigger count
  dataByte = trigcnt % 256; // modulus of 2^8
  for (int i=0; i < 8; i++){
    dataBits[i] = (dataByte & ( 1 << i )) >> i; // extract the i-th bit's 0 or 1 value, with help from https://stackoverflow.com/questions/2249731/how-do-i-get-bit-by-bit-data-from-an-integer-value-in-c
  }

  // Write all eight LED data outputs to GPIO
  digitalWriteFast(updatingLedPin, HIGH);  // Indicate that data is now being updated
  delay(1000);                  // wait for a second
  for (int i=0; i < 8; i++){
    digitalWriteFast(dataLedPins[i], dataBits[i]);   // set the LED value
  }
  digitalWriteFast(updatingLedPin, LOW);  // Indicate that data updates are complete

  // Print debug message over Serial
  Serial.print("dataByte: ");
  Serial.println(dataByte);

  Serial.print("dataBits: ");
  for (int i=0; i < 8; i++){
    Serial.print(dataBits[i]);
  }
  Serial.println("");

  // Blink the heartbeat LED once
  digitalWriteFast(heartbeatLedPin, HIGH);   // set the heartbeat LED on
  delay(500);                  // wait for a second
  digitalWriteFast(heartbeatLedPin, LOW);    // set the heartbeat LED off
  delay(500);                  // wait for a second

  trigcnt++;
}

/* Eight LEDs. Should maintain an array of eight LEDs.
 *  Eventual goal is to display data on these LEDs.
 *  
 *  Followed IntervalTimer tutorial at: https://www.pjrc.com/teensy/td_timing_IntervalTimer.html
 *  
*/

#define REF_HZ 20
#define REF_PIN 19
#define EXTTRIG_PIN 23

const int khzRefPin = 15; // Will output a kHz reference signal

const int builtInLedPin = 13; // On-board LED, only needed for troubleshooting
const int heartbeatLedPin = 3; // Pin for the heartbeat signal
const int updatingLedPin = 4; // Pin that is active only during the short period when data LEDs are in process of being updated

//const int dataLedPins[8] = {5,6,7,8,9,10,11,12}; // Use pins 5-12 as the one byte of data display
const int dataLedPins[8] = {12,11,10,9,8,7,6,5}; // Use pins 5-12 as the one byte of data display
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
  digitalWriteFast(updatingLedPin, HIGH);  // Indicate that data LEDs are now being updated
  for (int i=0; i < 8; i++){
    digitalWriteFast(dataLedPins[i], dataBits[i]);   // set the LED value
  }
  digitalWriteFast(updatingLedPin, LOW);  // Indicate that data LED updates are complete

  delayMicroseconds(200); // leave data active on LEDs for 200 microseconds

  // Shut off all eight data LEDs
  digitalWriteFast(updatingLedPin, HIGH);  // Indicate that data LEDs are now being updated
  for (int i=0; i < 8; i++){
    digitalWriteFast(dataLedPins[i], LOW);   // set the LED value
  }
  digitalWriteFast(updatingLedPin, LOW);  // Indicate that data LED updates are complete
}


void setup() {
  // Begin a 1 kHz reference signal
  analogWriteFrequency(REF_PIN, REF_HZ); // Set the frequency of the reference pulse
  analogWrite(REF_PIN, 26); // Creates a reference pulse of duty cycle 26/255 = 10%
  
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

  // attach the external interrupt
  pinMode(EXTTRIG_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(EXTTRIG_PIN), ISR_exttrig, RISING);
}

// the loop() methor runs over and over again,
// as long as the board has power

void loop() {
  // Blink the heartbeat LED once
  digitalWriteFast(heartbeatLedPin, HIGH);   // set the heartbeat LED on
  delay(500);                  // wait for a second
  digitalWriteFast(heartbeatLedPin, LOW);    // set the heartbeat LED off
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

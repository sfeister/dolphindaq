/* TFT
    Displays image data on a TFT from external USB source, like a Pi 5.

    Built for a Teensy 4.1 board

    PINS:
      23    |   Input, external trigger
      built-in LED     |   Output, green status LED, heartbeat. Flashes once per second reliably.
      4    |   Output, Debug (configured as needed)
            
    Followed TeensyTimerTool tutorial at: https://forum.pjrc.com/threads/59112-TeensyTimerTool

    Written by Scott Feister on May 3, 2025. Modified from my earlier device, DiodeSerial.ino.
    Dual Serial approach so I can send data at faster rates via PVaccess.
    Make sure to set up for Dual USB in Arduino IDE menu bar: "Tools" --> "USB Type" --> "Dual Serial"
*/

#include <Arduino.h>
#include <Chrono.h> //non-interrupt, low-precision timing
#include "scpi-def.h"
#include <TeensyTimerTool.h>
using namespace TeensyTimerTool;
#include <ddaqproto.h> // stub
#include <pb_encode.h>
#include <pb_decode.h>
#include <hello.pb.h> // custom protobuffer
#include <tft.pb.h> // custom protobuffer

#define DEBUG_PIN 4 // just for debugging
#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN LED_BUILTIN
#define IMAGE_NX 240
#define IMAGE_NY 340

volatile uint16_t imgbuf_display[IMAGE_NX*IMAGE_NY];

volatile uint64_t trigcnt = 0; // Trigger ID upcounter; increments with each external trigger rising edge, whether or not an image is acquired/transmitted
volatile uint64_t trigcnt_image = 0; // Trigger ID that matches the trace held in trace (until it's filled into the trace object)
volatile bool trigd = false; // is the system triggered (and the shot alert not yet raised)
volatile uint64_t trigtime = 0;

// Instantiate a Chrono object for the led heartbeat and LED trigger display
Chrono heartbeatChrono; 

/** Initialize TFT ShotAlert Protobuffer **/
uint8_t shotalert_buf[1024];
uint32_t shotalert_len;
bool shotalert_status;

dolphindaq_tft_ShotAlert shotalert = dolphindaq_tft_ShotAlert_init_zero;
pb_ostream_t shotalert_stream;

/** Initialize TFT Image Protobuffer **/
uint8_t image_buf[IMAGE_NX*IMAGE_NY*2 + 1024]; // TODO: Make this array longer
uint32_t image_len;
bool image_status;
dolphindaq_tft_Image image = dolphindaq_tft_Image_init_zero;
pb_istream_t image_stream;

// external trig callback interrupt service routine
void ISR_exttrig() {
    // (1) Increment trigger counter
    trigcnt++;
    trigtime = micros();

    // (2) Raise a shot alert
    trigd = true;
}

void send_shot_alert() {
  // Note: This function will block if called at too high rep-rate (if the USB isn't reading fast enough)
#if defined(USB_TRIPLE_SERIAL)
  uint32_t startTime = micros();
#endif

  shotalert.has_shot_num = true;
  shotalert.shot_num = trigcnt;

  shotalert_stream = pb_ostream_from_buffer(shotalert_buf, sizeof(shotalert_buf));
  shotalert_status = pb_encode(&shotalert_stream, dolphindaq_tft_ShotAlert_fields, &shotalert);
  shotalert_len = shotalert_stream.bytes_written;
#if defined(USB_DUAL_SERIAL) || defined(USB_TRIPLE_SERIAL)
  if (SerialUSB1.dtr()) { // only send the alert if anyone is listening for alerts
    SerialUSB1.println("shot alert");
    SerialUSB1.println(shotalert_len); // note: will block if no space, such that it can eventually send
    SerialUSB1.write(shotalert_buf, shotalert_len);
    SerialUSB1.send_now(); // schedule transmission immediately rather than waiting up to 5 ms for USB buffer to fill
#if defined(USB_TRIPLE_SERIAL)
    uint32_t endTime = micros();
    SerialUSB2.print("Scheduled data send. Time elapsed in the 'send_shot_alert()' blocking code of the ISR: ");
    SerialUSB2.print(endTime - startTime);
    SerialUSB2.println(" microseconds."); // I've found this to be taking under five microseconds.
#endif

  }
#endif
}

void setup() {
  pinMode(DEBUG_PIN, OUTPUT);

  // initialize the heartbeat LED and updating LED as output
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);

#if defined(USB_DUAL_SERIAL) || defined(USB_TRIPLE_SERIAL)
  SerialUSB1.println("the quick brown fox jumps over the lazy dog"); // a unique phrase to start transmission
#endif

  // attach the external interrupt
  pinMode(EXTTRIG_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(EXTTRIG_PIN), ISR_exttrig, RISING);
  NVIC_SET_PRIORITY(IRQ_GPIO6789, 0); // set all external interrupts to maximum priority level
  // Note that all interrupts using IRQ_GPIO6789 according to https://forum.pjrc.com/index.php?threads/teensy-4-set-interrupt-priority-on-given-pins.59828/post-231312

  // Initialize SCPI interface
  SCPI_Arduino_Setup(); // note, begins Serial if communication style is Serial
}

void loop() {
  // digitalToggleFast(DEBUG_PIN); // DEBUG
  // Blink the heartbeat LED
  if (heartbeatChrono.hasPassed(500)) { // check if the 500 milliseconds of heartbeat have elapsed
    digitalToggleFast(HEARTBEAT_LED_PIN); // write LED state to pin
    heartbeatChrono.restart();
#if defined(USB_TRIPLE_SERIAL)
    SerialUSB2.print("Trigger count: ");
    SerialUSB2.print(trigcnt);
    SerialUSB2.println("");
#endif

  }

  if (trigd){ // if we haven't alerted the most recent trigger count, it's time to send that
    trigd = false;
    send_shot_alert();
  }

#if defined(USB_DUAL_SERIAL) || defined(USB_TRIPLE_SERIAL)
  // Receive updated image data
  if (SerialUSB1.available() > 0) {
    // expects a payload byte length followed by the payload
    int image_len = SerialUSB1.readStringUntil('\n').toInt(); // read message length
    SerialUSB1.readBytes(image_buf, image_len); // read message payload
    
    // Decode message payload into the "image" object
    image_stream = pb_istream_from_buffer(image_buf, image_len);
    image_status = pb_decode(&image_stream, dolphindaq_tft_Image_fields, &image);

    // Print the shot number and round trip time to show this worked.
    int roundtrip_micros = micros() - trigtime; // I've found the roundtrip takes about 2 milliseconds.
#if defined(USB_TRIPLE_SERIAL)
    SerialUSB2.print("Image.shot_num: ");
    SerialUSB2.println(image.shot_num);
    SerialUSB2.print("Microseconds elapsed in the round trip: ");
    SerialUSB2.println(roundtrip_micros);
#endif

  }
#endif

  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}


/* TFT
    Displays image data on a TFT from external USB source, like a Jetson or GPU machine.

    Built for a Teensy 4.1 board

    PINS:
      23    |   Input, external trigger
      built-in LED     |   Output, green status LED, heartbeat. Flashes once per second reliably.
      4    |   Output, Debug (configured as needed)
            
    Followed TeensyTimerTool tutorial at: https://forum.pjrc.com/threads/59112-TeensyTimerTool

    TODO Wishlist:
    * Add in a variable delay that the image stays posted after being received. Could be handled with timers at trigger received.

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
#include <tft.pb.h> // custom protobuffer

/* BEGIN WAVESHARE HEADERS */
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
/* END WAVESHARE HEADERS */


/* BEGIN WAVESHARE DEFINES */
#define TFT_BL 9 // backlight pin
#define TFT_CS 10
#define TFT_DC 7
#define TFT_RST 8
/* END WAVESHARE DEFINES */

#define DEBUG_PIN 4 // just for debugging
#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN LED_BUILTIN
#define IMAGE_NX 280
#define IMAGE_NY 240
#define MAX_LATENCY_MILLIS 400 // maximum allowable latency between trigger arrival and image received back to Teensy, in milliseconds (otherwise screen will stay black)
// TODO: Make MAX_LATENCY_MILLIS configurable over SCPI
#define BLANK_MILLIS 600 // time after which the image gets blanked back out

#if !defined(USB_DUAL_SERIAL) && !defined(USB_TRIPLE_SERIAL)
#  error "Dual Serial USB must be set in Arduino IDE menu bar: Tools --> USB Type --> Dual Serial"
#endif

volatile uint16_t imgbuf_display[IMAGE_NX*IMAGE_NY];

volatile uint64_t trigcnt = 0; // Trigger ID upcounter; increments with each external trigger rising edge, whether or not an image is acquired/transmitted
volatile uint64_t trigcnt_alert = 0; // Trigger ID that matches the value that will be sent in the alert message
volatile uint64_t trigtime = 0; // Trigger timestamp (milliseconds, local to this device)
volatile uint64_t trigtime_alert = 0; // Trigger timestamp that matches the trigger alert
volatile bool trigd = false; // is the system triggered (e.g. in process of handling a trigger)
volatile bool alert_ready = false; // is there a shot alert ready to be sent out
volatile bool awaiting_response = false; // are we waiting on a response from the PC?

// Instantiate a Chrono object for the led heartbeat, internal trigger (as desired), and blanking screen
Chrono heartbeatChrono; 
Chrono internalTrigger; 
Chrono blanker; // used to blank out the screen 

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
dolphindaq_tft_ImageWaveshare image = dolphindaq_tft_ImageWaveshare_init_zero; // WAVESHARE SPECIFIC
pb_istream_t image_stream;

/* BEGIN WAVESHARE DECLARATIONS */
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
uint16_t framebuf[280][240];
/* END WAVESHARE DECLARATIONS */

// external trig callback interrupt service routine
void ISR_exttrig() {
    // (1) Increment trigger counter
    trigcnt++;
    trigtime = millis();

    // (2) Raise a shot alert
    if (!trigd) {
      trigd = true;
      trigcnt_alert = trigcnt;
      trigtime_alert = trigtime;
      alert_ready = true;
    }
}

void send_shot_alert() {
  // Note: This function will block if called at too high rep-rate (if the USB isn't reading fast enough)
#if defined(USB_TRIPLE_SERIAL)
  uint32_t startTime = micros();
#endif

  shotalert.has_shot_num = true;
  shotalert.shot_num = trigcnt_alert;

  shotalert_stream = pb_ostream_from_buffer(shotalert_buf, sizeof(shotalert_buf));
  shotalert_status = pb_encode(&shotalert_stream, dolphindaq_tft_ShotAlert_fields, &shotalert);
  shotalert_len = shotalert_stream.bytes_written;

  if (SerialUSB1.dtr()) { // only send the alert if anyone is listening for alerts
    SerialUSB1.println("shot alert");
    SerialUSB1.println(shotalert_len); // note: will block if no space, such that it can eventually send
    SerialUSB1.write(shotalert_buf, shotalert_len);
    SerialUSB1.send_now(); // schedule transmission immediately rather than waiting up to 5 ms for USB buffer to fill
    awaiting_response = true; // we will now wait for the next response

#if defined(USB_TRIPLE_SERIAL)
    uint32_t endTime = micros();
    SerialUSB2.print("Scheduled data send. Time elapsed in the 'send_shot_alert()' blocking code of the ISR: ");
    SerialUSB2.print(endTime - startTime);
    SerialUSB2.println(" microseconds."); // I've found this to be taking under five microseconds.
#endif    
  } else {
    trigd = false; // we never sent out a shot alert, so we won't expect any reply.
  }

}

/* BEGIN WAVESHARE FUNCTIONS */
void blank_tft_image() {
    tft.fillScreen(ST77XX_BLACK);
}

void update_tft_image() {
  tft.startWrite();
  tft.writePixels(reinterpret_cast<uint16_t*>(&image.vals[0]), 280*240);
  tft.endWrite();
}
/* END WAVESHARE FUNCTIONS */

void setup() {
  pinMode(DEBUG_PIN, OUTPUT);

  // initialize the heartbeat LED and updating LED as output
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);

  SerialUSB1.println("the quick brown fox jumps over the lazy dog"); // a unique phrase to start transmission

  // attach the external interrupt
  pinMode(EXTTRIG_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(EXTTRIG_PIN), ISR_exttrig, RISING);
  NVIC_SET_PRIORITY(IRQ_GPIO6789, 0); // set all external interrupts to maximum priority level
  // Note that all interrupts using IRQ_GPIO6789 according to https://forum.pjrc.com/index.php?threads/teensy-4-set-interrupt-priority-on-given-pins.59828/post-231312

  /* BEGIN WAVESHARE SETUP */
  tft.init(240, 280);   // Init ST7789 280x240
  tft.fillScreen(ST77XX_BLUE);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  /* END WAVESHARE SETUP */


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
    SerialUSB2.print("Trigger Count: ");
    SerialUSB2.print(trigcnt);
    SerialUSB2.print(", Last Alert: ");
    SerialUSB2.println(trigcnt_alert);
#endif
  }

  if (blanker.hasPassed(BLANK_MILLIS)) {
    blank_tft_image(); // clear out the old image on the screen from the last trigger
    blanker.restart();
  }


  if (internalTrigger.hasPassed(1500)) {
    ISR_exttrig();
    internalTrigger.restart();
  }

  if (!SerialUSB1.dtr()) { // device isn't even connected any more, give up on sending anything or awaiting a response
    awaiting_response = false;
    trigd = false;
    alert_ready = false;
  }

  if (alert_ready){ // if we haven't alerted the most recent trigger count, it's time to send that
    alert_ready = false;
    send_shot_alert();
    blank_tft_image(); // clear out the old image on the screen from the last trigger
    blanker.restart(); // don't blank out again for a bit
  }

  if (awaiting_response) {
    if (SerialUSB1.available() > 0) { // receive the image
      // expects a payload byte length followed by the payload
      int image_len = SerialUSB1.readStringUntil('\n').toInt(); // read message length
      SerialUSB1.readBytes(image_buf, image_len); // read message payload
      
      awaiting_response = false;

      // Decode message payload into the "image" object
      image_stream = pb_istream_from_buffer(image_buf, image_len);
      image_status = pb_decode(&image_stream, dolphindaq_tft_ImageWaveshare_fields, &image); // WAVESHARE SPECIFIC

      // Print the shot number and round trip time to show this worked.
      int roundtrip_millis = millis() - trigtime_alert; // I've found the roundtrip takes about 2 milliseconds.
#if defined(USB_TRIPLE_SERIAL)
      SerialUSB2.print("Image.shot_num: ");
      SerialUSB2.println(image.shot_num);
      SerialUSB2.print("Milliseconds elapsed between trigger and completion: ");
      SerialUSB2.println(roundtrip_millis);
      SerialUSB2.print("Image value at index 152 ");
      SerialUSB2.println(image.vals[152]);
#endif

      // Update the image on the TFT
      if (roundtrip_millis < MAX_LATENCY_MILLIS) {
        update_tft_image();
      } else {
#if defined(USB_TRIPLE_SERIAL)
        SerialUSB2.println("Image frame lost.");
#endif
      }

      trigd = false;
    }

  } else {
    // Keep our receive buffer clear if we aren't awaiting any response, to clear out any dangling data from start/restarts
    while (SerialUSB1.available() > 0) {
      SerialUSB1.read(); // Read and discard each byte in the buffer
    }
  }

  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}


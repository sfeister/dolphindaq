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

// Instantiate a Chrono object for the led heartbeat and LED trigger display
Chrono heartbeatChrono; 

/** Initialize Hello Protobuffer **/
uint8_t hello_buf[256];
uint32_t hello_len;
bool hello_status;

dolphindaq_Hello hello = dolphindaq_Hello_init_zero;
pb_ostream_t hello_stream;

/** Initialize Diode (Trace) Data Protobuffer **/
uint8_t image_buf[IMAGE_NX*IMAGE_NY*2 + 1024]; // TODO: Make this array longer
uint32_t image_len;
bool image_status;

dolphindaq_tft_Image image = dolphindaq_tft_Image_init_zero;
pb_ostream_t image_stream;

// external trig callback interrupt service routine
void ISR_exttrig() {
    // (1) Increment trigger counter
    trigcnt++;

    // (2) TODO: Send a trigger count message over serial
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

  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}


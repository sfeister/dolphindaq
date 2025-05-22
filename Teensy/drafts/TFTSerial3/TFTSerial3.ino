/* TFT
    Displays image data on a TFT from external USB source, like a Jetson or GPU machine.

    Built for a Teensy 4.1 board

    PINS:
      23    |   Input, external trigger
      built-in LED     |   Output, green status LED, heartbeat. Flashes once per second reliably.
      4    |   Output, Debug (configured as needed)
            
    Followed TeensyTimerTool tutorial at: https://forum.pjrc.com/threads/59112-TeensyTimerTool

    Written by Scott Feister on May 8, 2025. Modified from my earlier device, DiodeSerial.ino.
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

/* BEGIN TFT HEADERS */
#include <ILI9341_T4.h>
/* END TFT HEADERS */


/* BEGIN TFT DEFINES */
//
// DEFAULT WIRING USING SPI 0 ON TEENSY 4/4.1
//
#define PIN_BACKLIGHT 255   // optional, set this only if the screen LED pin is connected directly to the Teensy. Connect with resistor.
#define PIN_MISO    12      // mandatory  (if the display has no MISO line, set this to 255 but then VSync will be disabled...)
#define PIN_SCK     13      // mandatory
#define PIN_MOSI    11      // mandatory
#define PIN_DC      10      // mandatory, can be any pin but using pin 10 (or 36 or 37 on T4.1) provides greater performance
#define PIN_RESET   6       // optional (but recommended), can be any pin. 
#define PIN_CS      9       // optional (but recommended), can be any pin.
// GND --> GND on Teensy 4
// VCC --> 3V on Teensy 4
#define PIN_TOUCH_IRQ 255   // optional. set this only if the touchscreen is connected on the same SPI bus
#define PIN_TOUCH_CS  255   // optional. set this only if the touchscreen is connected on the same spi bus

#define SPI_SPEED       100000000 // faster (up to 100 MHz) reduces tearing for full image redraws

/* END TFT DEFINES */

#define DEBUG_PIN 4 // just for debugging
#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN LED_BUILTIN
#define IMAGE_NX 320
#define IMAGE_NY 240
#define MAX_LATENCY_MILLIS 50 // maximum allowable latency between trigger arrival and image received back to Teensy, in milliseconds (otherwise screen will stay black)
// TODO: Make MAX_LATENCY_MILLIS configurable over SCPI
#define BLANK_MILLIS 100 // time after which the image gets blanked back out
#define INTERNAL_TRIG_MILLIS 175 // milliseconds between internal triggers (not an exact science here!)

#if !defined(USB_DUAL_SERIAL) && !defined(USB_TRIPLE_SERIAL)
#  error "Dual Serial USB must be set in Arduino IDE menu bar: Tools --> USB Type --> Dual Serial"
#endif

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
dolphindaq_tft_ImageILI image = dolphindaq_tft_ImageILI_init_zero; // TFT SPECIFIC
pb_istream_t image_stream;

/* BEGIN TFT DECLARATIONS */
// the screen driver object
ILI9341_T4::ILI9341Driver tft(PIN_CS, PIN_DC, PIN_SCK, PIN_MOSI, PIN_MISO, PIN_RESET, PIN_TOUCH_CS, PIN_TOUCH_IRQ);

// 2 diff buffers with about 6K memory each
// (in this simple case, only 1K memory buffer would be enough). 
ILI9341_T4::DiffBuffStatic<6000> diff1;
ILI9341_T4::DiffBuffStatic<6000> diff2;

// screen dimension 
const int LX = 320; 
const int LY = 240;
//const int LX = 20; 
//const int LY = 20;

// framebuffers
DMAMEM uint16_t internal_fb[LX*LY];     // used for buffering
DMAMEM uint16_t fb[LX*LY];              // main framebuffer we draw onto.
/* END TFT DECLARATIONS */

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

/* BEGIN TFT FUNCTIONS */
/** fill a framebuffer with a given color */
void clear(uint16_t* fb, uint16_t color = 0)
    {
    for (int i = 0; i < LX * LY; i++) fb[i] = color;
    }


/** draw a disk centered at (x,y) with radius r and color col on the framebuffer fb */
void drawDisk(uint16_t* fb, double x, double y, double r, uint16_t col)
    {
    int xmin = (int)(x - r);
    int xmax = (int)(x + r);
    int ymin = (int)(y - r);
    int ymax = (int)(y + r);
    if (xmin < 0) xmin = 0;
    if (xmax >= LX) xmax = LX - 1;
    if (ymin < 0) ymin = 0;
    if (ymax >= LY) ymax = LY - 1;
    const double r2 = r * r;
    for (int j = ymin; j <= ymax; j++)
        {
        double dy2 = (y - j) * (y - j);
        for (int i = xmin; i <= xmax; i++)
            {
            const double dx2 = (x - i) * (x - i);
            if (dx2 + dy2 <= r2) fb[i + (j * LX)] = col;
            }
        }
    }

void blank_tft_image() {
  clear(fb, ILI9341_T4_COLOR_BLACK); // draw a black background
  //tft.overlayFPS(fb, 1); // draw fps counter on bottom right
  tft.overlayText(fb, "Scott special demo", 3, 0, 12, ILI9341_T4_COLOR_WHITE, 1.0f, ILI9341_T4_COLOR_RED, 0.4f, 1); // draw text    
  tft.update(fb); // push the framebuffer to be displayed
}

void update_tft_image() {
  // Copy data from image object into framebuffer object
  // TODO: Use memcopy or something easier
  for (int i = 0; i < LX * LY; i++) fb[i] = *reinterpret_cast<uint16_t*>(&image.vals[i*2]);
  //tft.overlayFPS(fb, 1); // draw fps counter on bottom right
  char buffer[20];
  int base = 10;
  tft.overlayText(fb, itoa(image.shot_num, buffer, base), 3, 0, 12, ILI9341_T4_COLOR_WHITE, 1.0f, ILI9341_T4_COLOR_RED, 0.4f, 1); // draw text    
  tft.update(fb); // push the framebuffer to be displayed
}



/* END TFT FUNCTIONS */

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

  /* BEGIN TFT SETUP */
#if defined(USB_TRIPLE_SERIAL)
  tft.output(&SerialUSB2);                // output debug infos to serialUSB2 port.  
#endif
  while (!tft.begin(SPI_SPEED));      // init the display
  
  tft.setRotation(0);                 // portrait mode 240 x320
  tft.setFramebuffer(internal_fb);    // set the internal framebuffer (enables double buffering)
  tft.setDiffBuffers(&diff1, &diff2); // set the 2 diff buffers => activate differential updates. 
  tft.setDiffGap(4);                  // use a small gap for the diff buffers

  tft.setRefreshRate(120);            // around 120hz for the display refresh rate. 
  tft.setVSyncSpacing(2);             // set framerate = refreshrate/2 (and enable vsync at the same time). 
  //tft.setVSyncSpacing(0);  // disable vsync => would create screen tearing if we were moving fast enough

  if (PIN_BACKLIGHT != 255)
      { // make sure backlight is on
      pinMode(PIN_BACKLIGHT, OUTPUT);
      digitalWrite(PIN_BACKLIGHT, HIGH);
      }
  
  /* END TFT SETUP */


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
    //tft.printStats  ();
  }

  if (blanker.hasPassed(BLANK_MILLIS)) {
    blank_tft_image(); // clear out the old image on the screen from the last trigger
    blanker.restart();
  }


  if (internalTrigger.hasPassed(INTERNAL_TRIG_MILLIS)) {
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
      // Print the shot number and round trip time to show this worked.
      int roundtrip_millis = millis() - trigtime_alert; // I've found the roundtrip takes about 2 milliseconds.

      // expects a payload byte length followed by the payload
      int image_len = SerialUSB1.readStringUntil('\n').toInt(); // read message length
      SerialUSB1.readBytes(image_buf, image_len); // read message payload
      
      awaiting_response = false;

      // Decode message payload into the "image" object
      image_stream = pb_istream_from_buffer(image_buf, image_len);
      image_status = pb_decode(&image_stream, dolphindaq_tft_ImageILI_fields, &image); // TFT SPECIFIC

      // 
      int latency_millis = millis() - trigtime_alert; // I've found the roundtrip takes about 2 milliseconds.

      // Update the image on the TFT
      if (latency_millis < MAX_LATENCY_MILLIS) {
        update_tft_image();
      } else {
#if defined(USB_TRIPLE_SERIAL)
        SerialUSB2.println("Image frame lost.");
#endif
      }
      int total_millis = millis() - trigtime_alert; // I've found the roundtrip takes about 2 milliseconds.

#if defined(USB_TRIPLE_SERIAL)
      SerialUSB2.print("Image.shot_num: ");
      SerialUSB2.println(image.shot_num);
      SerialUSB2.print("Milliseconds elapsed between trigger and data receipt: ");
      SerialUSB2.println(roundtrip_millis);
      SerialUSB2.print("Milliseconds elapsed between trigger and pb decode: ");
      SerialUSB2.println(latency_millis);
      SerialUSB2.print("Milliseconds elapsed between trigger and TFT update: ");
      SerialUSB2.println(total_millis);
      SerialUSB2.print("Image value at index 152 ");
      SerialUSB2.println(image.vals[152]);
#endif

      trigd = false;
    }

  } else {
    // Keep our receive buffer clear if we aren't awaiting any response, to clear out any dangling data from start/restarts
    while (SerialUSB1.available() > 0) {
      SerialUSB1.read(); // Read and discard each byte in the buffer
    }
  }

  //tft.update(fb); // push the framebuffer to be displayed
  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}


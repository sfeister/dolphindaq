/* DiodeTFT
    Records data from a phototransistor, posts it over USB, receives image data back, and displays image.

    Built for a Teensy 4.1 board

    PINS:
      23    |   Input, external trigger
      14 (A0) |   Input, phototransistor pin
      built-in LED     |   Output, green status LED, heartbeat. Flashes once per second reliably.
      4    |   Output, Debug (configured as needed)
            
    Followed TeensyTimerTool tutorial at: https://forum.pjrc.com/threads/59112-TeensyTimerTool
    Followed (and copied portions of code from) ADC tutorials at https://github.com/pedvide/ADC

    Written by Scott Feister on October 11, 2023.
    On July 24, 2024, updated to a Dual Serial approach so I can send data at faster rates via PVaccess.
    Make sure to set up for Dual USB in Arduino IDE menu bar: "Tools" --> "USB Type" --> "Dual Serial"
    Dt was tested down to 1 microsecond and works fine
    Drift in start/end time of aquisition will be proportional to dt, since ADC conversions happen continuously
*/

#include <Arduino.h>
#include <Chrono.h> //non-interrupt, low-precision timing
#include "scpi-def.h"
#include <TeensyTimerTool.h>
using namespace TeensyTimerTool;
#include <ddaqproto.h> // stub
#include <pb_encode.h>
#include <pb_decode.h>
#include <ADC.h> // from https://github.com/pedvide/ADC
#include <hello.pb.h> // custom protobuffer
#include <diode.pb.h> // custom protobuffer
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

#define IMAGE_NX 320
#define IMAGE_NY 240
#define MAX_LATENCY_MILLIS 50 // maximum allowable latency between trigger arrival and image received back to Teensy, in milliseconds (otherwise screen will stay black)
// TODO: Make MAX_LATENCY_MILLIS configurable over SCPI
#define BLANK_MILLIS 100 // time after which the image gets blanked back out
#define INTERNAL_TRIG_MILLIS 175 // milliseconds between internal triggers (not an exact science here!)

/* END TFT DEFINES */

#define DEBUG_PIN 4 // just for debugging
#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN LED_BUILTIN
#define TRACE_NT 100

#if !defined(USB_DUAL_SERIAL) && !defined(USB_TRIPLE_SERIAL)
#  error "Dual Serial USB must be set in Arduino IDE menu bar: Tools --> USB Type --> Dual Serial"
#endif

OneShotTimer t1;

// ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino
// Following adc_timer_dma_oneshot.ino example from ADC
#include <AnalogBufferDMA.h>
#include <DMAChannel.h>
const int readPin_adc_0 = A0;
ADC *adc = new ADC(); // adc object
extern void dumpDMA_structures(DMABaseClass *dmabc);

const uint32_t buffer_size = TRACE_NT;
DMAMEM static volatile uint16_t __attribute__((aligned(32)))
dma_adc_buff1[buffer_size];
AnalogBufferDMA abdma1(dma_adc_buff1, buffer_size);
// END ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino

volatile uint16_t imgbuf_trace[TRACE_NT];


volatile bool lock_adc = false; // Boolean: 1 if adc_buffer is currently being written to
volatile bool lock_trace = false; // Boolean: 1 if trace is currently being written to
volatile bool ripe_trace = false; // Boolean: 1 if Trace block is filled and ready (a new trace is "ripe" for transfer)
volatile bool await_update = false; // Boolean: 1 if there are new updates to the device settings waiting to be implemented, else 0
volatile uint64_t trigcnt = 0; // Trigger ID upcounter; increments with each external trigger rising edge, whether or not an image is acquired/transmitted
volatile uint64_t trigcnt_adc = 0; // Trigger ID that matches the trace held in adc_buf array, updated prior to buffer being filled
volatile uint64_t trigcnt_trace = 0; // Trigger ID that matches the trace held in trace (until it's filled into the trace object)
volatile uint16_t imgbuf_adc[TRACE_NT]; // Buffer completely re-filled by the ADC during each acquisition. Used within the ADC block of the timing.

// TFT Specific
volatile uint64_t trigtime_alert = 0; // Trigger timestamp that matches the trigger alert
volatile bool trigd = false; // is the system triggered (e.g. in process of handling a trigger)
volatile bool awaiting_response = false; // are we waiting on a response from the PC?

// Instantiate a Chrono object for the led heartbeat and LED trigger display
Chrono heartbeatChrono; 
Chrono blanker; // used to blank out the screen 

/** Initialize Hello Protobuffer **/
uint8_t hello_buf[256];
uint32_t hello_len;
bool hello_status;

dolphindaq_Hello hello = dolphindaq_Hello_init_zero;
pb_ostream_t hello_stream;

/** Initialize Diode Settings Protobuffer **/
uint8_t settings_buf[256]; // TODO: Make this array longer
uint32_t settings_len;
bool settings_status;

dolphindaq_diode_Settings settings = dolphindaq_diode_Settings_init_zero;
dolphindaq_diode_Settings settings_pr = dolphindaq_diode_Settings_init_zero; // preload settings (currently unused)
pb_ostream_t settings_stream;

/** Initialize Diode (Trace) Data Protobuffer **/
uint8_t data_buf[TRACE_NT*2 + 1024]; // TODO: Make this array much longer
uint32_t data_len;
bool data_status;

dolphindaq_diode_Data data = dolphindaq_diode_Data_init_zero;
pb_ostream_t data_stream;

dolphindaq_diode_Trace trace = dolphindaq_diode_Trace_init_zero;

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

    // (2) Activate the DMA from the continuous ADC
    if ((!lock_adc) && (!trigd) && (!await_update)) {
      lock_adc = true;
      trigd = true;
      trigcnt_adc = trigcnt;
      trigtime_alert = millis();
      abdma1.clearCompletion(); // start DMA
    }
}

// Interrupt to be called at the end of DMA transfer
void ISR_abdma1() {
    abdma1.adc_0_dmaISR(); // this is the original ISR that ours replaced, so call it first
    
    // Now we do some of our own stuff!
    ProcessAnalogData(&abdma1, 0);
    abdma1.clearInterrupt();
    lock_adc = false;
}

// every single ADC conversion completion (each element of array) will call this function
// void ISR_ADCconv() {
  // digitalToggleFast(DEBUG_PIN); // DEBUG
// }

// Fills in structured data of an already-created settings protobuffer
// Also serves to initialize the settings, in some cases
// Impose the settings (and update unreasonable settings)
void update_settings(dolphindaq_diode_Settings* settings_ptr) {
  dolphindaq_diode_Settings* s = settings_ptr; // shorthand

  // Get current shot number
  s->has_start_shot_num = true;
  s->start_shot_num = trigcnt;

  // Number of array elements (kept fixed for now, since this affects memory and stability requirements)
  s->has_trace_nt = true;
  s->trace_nt = TRACE_NT;

  if (!s->has_dt) {
    s->has_dt = true;
    s->dt = 5.0e-6; // default dt of 5 microseconds
  }
  s->has_trace_dt = true;
  s->trace_dt = s->dt;

  // Y axis limits
  s->has_trace_ymin = true;
  s->trace_ymin = 0.0; // TODO: Confirm that "0" in ADC actually matches this voltage (or grab it from the chip)
  s->has_trace_ymax = true;
  s->trace_ymax = 3.3; // TODO: Confirm this max voltage of 3.3 V (or grab it from the chip)
}

/* BEGIN TFT FUNCTIONS */
/** fill a framebuffer with a given color */
void clear(uint16_t* fb, uint16_t color = 0)
    {
    for (int i = 0; i < LX * LY; i++) fb[i] = color;
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

  // Fill in setting's structured data, and encode the protobuf message
  update_settings(&settings);
  settings_stream = pb_ostream_from_buffer(settings_buf, sizeof(settings_buf));
  settings_status = pb_encode(&settings_stream, dolphindaq_diode_Settings_fields, &settings);
  settings_len = settings_stream.bytes_written;
#if defined(USB_DUAL_SERIAL) || defined(USB_TRIPLE_SERIAL)
  SerialUSB1.println("the quick brown fox jumps over the lazy dog"); // a unique phrase to start transmission
  SerialUSB1.println("settings");
  //SerialUSB1.print(settings_len); // note: will block if no space, such that it can eventually send
  //SerialUSB1.write(settings_buf, settings_len);
#endif

  // ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino
  pinMode(readPin_adc_0, INPUT_DISABLE); // Not sure this does anything for us
  adc->adc0->setAveraging(0);   // set number of averages
  adc->adc0->setResolution(10); // set bits of resolution
  // adc->adc0->setConversionSpeed(ADC_settings::ADC_CONVERSION_SPEED::ADACK_20); // change the ADC Clock (ADCK) to the ADC asynchronous clock of 20 MHz (https://forum.pjrc.com/index.php?threads/adc-library-with-support-for-teensy-4-3-x-and-lc.25532/)
  adc->adc0->setConversionSpeed(ADC_settings::ADC_CONVERSION_SPEED::VERY_HIGH_SPEED); // Should be about 40 MHz (https://forum.pjrc.com/index.php?threads/adc-library-with-support-for-teensy-4-3-x-and-lc.25532/)
  adc->adc0->setSamplingSpeed(ADC_settings::ADC_SAMPLING_SPEED::VERY_HIGH_SPEED); // is the highest possible sampling speed (+0 ADCK, 2 in total. If ADCK is 20 MHz, this means each measurement takes 100 nanoseconds. For 32 averages that is 3.2 us)
  // setup a DMA Channel.
  // Now lets see the different things that RingbufferDMA setup for us before
  abdma1.init(adc, ADC_0 /*, DMAMUX_SOURCE_ADC_ETC*/);
  abdma1._dmachannel_adc.attachInterrupt(&ISR_abdma1); // put in our own interrupt

  // adc->adc0->enableInterrupts(&ISR_ADCconv, 255);

  // Start the dma operation..
  lock_adc = true;
  adc->adc0->startSingleRead(readPin_adc_0); // call this to setup everything before the Timer starts,
  adc->adc0->startTimer(static_cast<uint32_t>(1.0 / settings.dt)); // frequency in Hz

  // end ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino

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
    SerialUSB2.print("lock_adc: ");
    SerialUSB2.print(lock_adc);
    SerialUSB2.print(", lock_trace: ");
    SerialUSB2.print(lock_trace);
    SerialUSB2.print(", abdma1 interrupted: ");
    SerialUSB2.print(abdma1.interrupted());
    SerialUSB2.print(", abdma1 interrupt Count: ");
    SerialUSB2.print(abdma1.interruptCount());
    SerialUSB2.print(", abdma1 buffer Count: ");
    SerialUSB2.print(abdma1.bufferCountLastISRFilled());
    SerialUSB2.print(", abdma1 delta Time: ");
    SerialUSB2.print(abdma1.interruptDeltaTime());
    SerialUSB2.print(", trigcnt: ");
    SerialUSB2.print(trigcnt);
    SerialUSB2.print(", trigcnt_adc: ");
    SerialUSB2.print(trigcnt_adc);
    SerialUSB2.print(", trigcnt_trace: ");
    SerialUSB2.print(trigcnt_trace);
    SerialUSB2.println("");
#endif

  }

  // Check for settings updates
  if (await_update) {
    delayMicroseconds(100); // short delay to clear out any existing triggered pulses

    // Stop the ADC
    adc->adc0->stopTimer();

    // cancel any existing TFT activities
    awaiting_response = false;
    trigd = false;

    // Implement all updates
    update_settings(&settings);
    settings_stream = pb_ostream_from_buffer(settings_buf, sizeof(settings_buf));
    settings_status = pb_encode(&settings_stream, dolphindaq_diode_Settings_fields, &settings);
    settings_len = settings_stream.bytes_written;
#if defined(USB_DUAL_SERIAL) || defined(USB_TRIPLE_SERIAL)
    SerialUSB1.println("settings");
    SerialUSB1.println(settings_len);
    SerialUSB1.write(settings_buf, settings_len);
#endif

    // Restart the ADC with new settings
    adc->adc0->startSingleRead(readPin_adc_0); // call this to setup everything before the Timer starts,
    adc->adc0->startTimer(static_cast<uint32_t>(1.0 / settings.dt)); // frequency in Hz

    await_update = false;
  }
  
  /*
  if (abdma1.interrupted()) {
    digitalToggleFast(DEBUG_PIN); // DEBUG
    //digitalToggleFast(DEBUG_PIN, HIGH);
    ProcessAnalogData(&abdma1, 0);

    abdma1.clearInterrupt();
    lock_adc = false;
    //digitalWriteFast(DEBUG_PIN, LOW);
  }
  */

  if (ripe_trace) {
    blank_tft_image(); // clear out the old image on the screen from the last trigger
    blanker.restart(); // don't blank out again for a bit
    encode_trace(); // Transfers Trace block into Trace-Proto block, and also clears ripe_trace and lock_trace flags.
    // Transmit protobuf data over second serial port
#if defined(USB_DUAL_SERIAL) || defined(USB_TRIPLE_SERIAL)
    if ( (!SerialUSB1.dtr()) && (SerialUSB1.availableForWrite() > static_cast<int>(settings_len + data_len + 100)) ) {
      SerialUSB1.println("the quick brown fox jumps over the lazy dog"); // a unique phrase to reset transmission
      SerialUSB1.println("settings");
      SerialUSB1.println(settings_len); // note: will block if no space, such that it can eventually send
      SerialUSB1.write(settings_buf, settings_len);
      SerialUSB1.println("data");
      SerialUSB1.println(data_len);
      SerialUSB1.write(data_buf, data_len);
      awaiting_response = true;
    }
#endif
  }

  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}

// ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino (with some modifications now)
void ProcessAnalogData(AnalogBufferDMA *pabdma, int8_t adc_num) {
  //digitalWriteFast(DEBUG_PIN, HIGH); // DEBUG
  volatile uint16_t *pbuffer = pabdma->bufferLastISRFilled();
  //Serial.println(pabdma->bufferCountLastISRFilled());

  if ((uint32_t)pbuffer >= 0x20200000u)
    arm_dcache_delete((void *)pbuffer, sizeof(dma_adc_buff1));
  
  // Copy the buffer data into the image buffer
  if (!lock_trace) {
    lock_trace = true;
    trigcnt_trace = trigcnt_adc;
    for (int i = 0; i < TRACE_NT; i++) {
      imgbuf_trace[i] = *pbuffer;
      pbuffer++;
    }
    ripe_trace = true; // indicate that Trace block is filled and ready for usage
    //digitalWriteFast(DEBUG_PIN, LOW); // DEBUG
  }

  // TFT-related
  if (blanker.hasPassed(BLANK_MILLIS)) {
    blank_tft_image(); // clear out the old image on the screen from the last trigger
    blanker.restart();
  }

  if (!SerialUSB1.dtr()) { // device isn't even connected any more, give up on sending anything or awaiting a response
    awaiting_response = false;
    trigd = false;
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

}
// END ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino

// Works with the global variable imgbuf_trace and TRACE_NT
bool encode_imgbuf_trace(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
  if (!pb_encode_tag_for_field(stream, field))
      return false;

  return pb_encode_string(stream, const_cast<pb_byte_t*>(reinterpret_cast<volatile pb_byte_t*>(&imgbuf_trace)), TRACE_NT*sizeof(uint16_t));
}

// Fills in structured data of an already-created data_t protobuffer (data trace protobuffer)
void update_data_t(dolphindaq_diode_Data* data_ptr, dolphindaq_diode_Trace* trace_ptr) {
  data_ptr->has_trace = true;
  data_ptr->trace = *trace_ptr;
  data_ptr->trace.has_shot_num = true;
  data_ptr->trace.shot_num = trigcnt_trace;
  data_ptr->trace.yvals.funcs.encode = &encode_imgbuf_trace;
  // data_ptr->trace.has_shot_time = true;
  //data_ptr->trace.shot_time = to_timestamp(trigtime_trace);
}

// Encodes data from the Trace block into the Trace-Proto block, then unlocks the Trace block
void encode_trace() {
  /* Encode Trace within a Protobuffer "Data" message */
  update_data_t(&data, &trace);
  data_stream = pb_ostream_from_buffer(data_buf, sizeof(data_buf));
  data_status = pb_encode(&data_stream, dolphindaq_diode_Data_fields, &data);
  data_len = data_stream.bytes_written;

  // TODO: Encoding error handling
  ripe_trace = false; // reset the Trace block for update
  lock_trace = false; // 
}


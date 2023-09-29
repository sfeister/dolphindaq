/* Diode
    Records data from a phototransistor.

    Built for a Teensy 4.1 board

    PINS:
      23    |   Input, external trigger
      3     |   Output, green status LED, heartbeat. Flashes once per second reliably.
      19    |   Output, Debug. HIGH when an acquisition starts, LOW when the acquisition stops
            
    Followed TeensyTimerTool tutorial at: https://forum.pjrc.com/threads/59112-TeensyTimerTool
    
    Written by Scott Feister on August 4, 2023.
*/

#include <Arduino.h>
#include <Chrono.h> //non-interrupt, low-precision timing
#include "scpi-def.h"
#include <TeensyTimerTool.h>
using namespace TeensyTimerTool;
#include <pb_encode.h>
#include <pb_decode.h>
#include <ADC.h> // from https://github.com/pedvide/ADC
#include <hello.pb.h> // custom protobuffer
#include <diode.pb.h> // custom protobuffer

#define DEBUG_PIN 19 // reference / internal trigger pin
#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN 3
#define TRACE_NT 50

OneShotTimer t1;

// ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino
// Following adc_timer_dma_oneshot.ino example from ADC
#include <AnalogBufferDMA.h>
#include <DMAChannel.h>
const int readPin_adc_0 = A0;
ADC *adc = new ADC(); // adc object
const uint32_t initial_average_value = 2048;
extern void dumpDMA_structures(DMABaseClass *dmabc);

const uint32_t buffer_size = TRACE_NT;
DMAMEM static volatile uint16_t __attribute__((aligned(32)))
dma_adc_buff1[buffer_size];
AnalogBufferDMA abdma1(dma_adc_buff1, buffer_size);
// END ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino

uint16_t imgbuf_trace[TRACE_NT];


bool lock_adc = false; // Boolean: 1 if adc_buffer is currently being written to
bool lock_trace = false; // Boolean: 1 if trace is currently being written to
bool await_update = false; // Boolean: 1 if there are new updates to the device settings waiting to be implemented, else 0
uint64_t trigcnt = 0; // Trigger ID upcounter; increments with each external trigger rising edge, whether or not an image is acquired/transmitted
uint64_t trigcnt_adc = 0; // Trigger ID that matches the trace held in adc_buf array, updated prior to buffer being filled
uint64_t trigcnt_trace = 0; // Trigger ID that matches the trace held in trace (until it's filled into the trace object)
uint16_t imgbuf_adc[TRACE_NT]; // Buffer completely re-filled by the ADC during each acquisition. Used within the ADC block of the timing.

// Instantiate a Chrono object for the led heartbeat and LED trigger display
Chrono heartbeatChrono; 

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
dolphindaq_diode_Settings settings_pr = dolphindaq_diode_Settings_init_zero; // preload settings
pb_ostream_t settings_stream;

/** Initialize Diode (Trace) Data Protobuffer **/
uint8_t data_buf[TRACE_NT*2 + 1024]; // TODO: Make this array much longer
uint32_t data_len;
bool data_status;

dolphindaq_diode_Data data = dolphindaq_diode_Data_init_zero;
pb_ostream_t data_stream;

dolphindaq_diode_Trace trace = dolphindaq_diode_Trace_init_zero;

// external trig callback interrupt service routine
void ISR_exttrig() {
    // (1) Increment trigger counter
    trigcnt++;

    // (2) Activate the ADC (TODO)
    if (!lock_adc) {
      lock_adc = true;
      trigcnt_adc = trigcnt;
      abdma1.clearCompletion(); // run it again
    }
}

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

  // Y axis limits
  s->has_trace_ymin = true;
  s->trace_ymin = 0.0; // TODO: Confirm that "0" in ADC actually matches this voltage (or grab it from the chip)
  s->has_trace_ymax = true;
  s->trace_ymax = 3.3; // TODO: Confirm this max voltage of 3.3 V (or grab it from the chip)
}

void setup() {
  // initialize the heartbeat LED and updating LED as output
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);

  // ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino
  pinMode(readPin_adc_0, INPUT_DISABLE); // Not sure this does anything for us
  adc->adc0->setAveraging(0);   // set number of averages
  adc->adc0->setResolution(10); // set bits of resolution
  // setup a DMA Channel.
  // Now lets see the different things that RingbufferDMA setup for us before
  abdma1.init(adc, ADC_0 /*, DMAMUX_SOURCE_ADC_ETC*/);
  abdma1.userData(initial_average_value); // save away initial starting average

  // Start the dma operation..
  lock_adc = true;
  adc->adc0->startSingleRead(
      readPin_adc_0); // call this to setup everything before the Timer starts,
                      // differential is also possible
  adc->adc0->startTimer(100000); // frequency in Hz

  // end ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino

  // attach the external interrupt
  pinMode(EXTTRIG_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(EXTTRIG_PIN), ISR_exttrig, RISING);

  update_settings(&settings);

  // Initialize SCPI interface
  SCPI_Arduino_Setup(); // note, begins Serial if communication style is Serial
}

void loop() {
  // Blink the heartbeat LED
  if (heartbeatChrono.hasPassed(500)) { // check if the 500 milliseconds of heartbeat have elapsed
    digitalToggleFast(HEARTBEAT_LED_PIN); // write LED state to pin
    heartbeatChrono.restart();
  }

  // Check for settings updates
  if (await_update) {
    delayMicroseconds(100); // short delay to clear out any existing triggered pulses
    // TODO: Implement all updates
    await_update = false;
  }
  
  if (abdma1.interrupted()) {
    ProcessAnalogData(&abdma1, 0);
    lock_adc = false;
  }

  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}

// ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino
void ProcessAnalogData(AnalogBufferDMA *pabdma, int8_t adc_num) {
  volatile uint16_t *pbuffer = pabdma->bufferLastISRFilled();
  //Serial.println(pabdma->bufferCountLastISRFilled());

  if ((uint32_t)pbuffer >= 0x20200000u)
    arm_dcache_delete((void *)pbuffer, sizeof(dma_adc_buff1));
  
  // Copy the buffer data into the image buffer
  lock_trace = true;
  for (int i = 0; i < TRACE_NT; i++) {
    imgbuf_trace[i] = *pbuffer;
    pbuffer++;
  }
  lock_trace = false;
}
// END ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino


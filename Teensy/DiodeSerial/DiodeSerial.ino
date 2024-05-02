/* Diode
    Records data from a phototransistor.

    Built for a Teensy 4.1 board

    PINS:
      23    |   Input, external trigger
      14 (A0) |   Input, phototransistor pin
      built-in LED     |   Output, green status LED, heartbeat. Flashes once per second reliably.
      19    |   Output, Debug. HIGH when an acquisition starts, LOW when the acquisition stops
            
    Followed TeensyTimerTool tutorial at: https://forum.pjrc.com/threads/59112-TeensyTimerTool
    Followed (and copied portions of code from) ADC tutorials at https://github.com/pedvide/ADC

    Written by Scott Feister on October 11, 2023.

    MODIFIED FOR SIDEKICK MODEL 4 by SWITCHING PHOTOTRANSISTOR POLARITIES
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

#define DEBUG_PIN 4 // just for debugging
#define EXTTRIG_PIN 23
#define HEARTBEAT_LED_PIN LED_BUILTIN
#define TRACE_NT 100

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

volatile uint16_t imgbuf_trace[TRACE_NT];


volatile bool lock_adc = false; // Boolean: 1 if adc_buffer is currently being written to
volatile bool lock_trace = false; // Boolean: 1 if trace is currently being written to
volatile bool ripe_trace = false; // Boolean: 1 if Trace block is filled and ready (a new trace is "ripe" for transfer)
volatile bool await_update = false; // Boolean: 1 if there are new updates to the device settings waiting to be implemented, else 0
volatile uint64_t trigcnt = 0; // Trigger ID upcounter; increments with each external trigger rising edge, whether or not an image is acquired/transmitted
volatile uint64_t trigcnt_adc = 0; // Trigger ID that matches the trace held in adc_buf array, updated prior to buffer being filled
volatile uint64_t trigcnt_trace = 0; // Trigger ID that matches the trace held in trace (until it's filled into the trace object)
volatile uint16_t imgbuf_adc[TRACE_NT]; // Buffer completely re-filled by the ADC during each acquisition. Used within the ADC block of the timing.

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
dolphindaq_diode_Settings settings_pr = dolphindaq_diode_Settings_init_zero; // preload settings (currently unused)
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

    // (2) Activate the ADC
    if (!lock_adc) {
      lock_adc = true;
      trigcnt_adc = trigcnt;
      adc->adc0->startSingleRead(readPin_adc_0);
      adc->adc0->startTimer(static_cast<uint32_t>(1.0 / settings.dt)); // frequency in Hz
      digitalToggleFast(DEBUG_PIN);
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

  if (!s->has_dt) {
    s->has_dt = true;
    s->dt = 10.0e-6; // default dt of 10 microseconds
  }
  s->has_trace_dt = true;
  s->trace_dt = s->dt;

  // Y axis limits
  s->has_trace_ymin = true;
  s->trace_ymin = 0.0; // TODO: Confirm that "0" in ADC actually matches this voltage (or grab it from the chip)
  s->has_trace_ymax = true;
  s->trace_ymax = 3.3; // TODO: Confirm this max voltage of 3.3 V (or grab it from the chip)
}

void setup() {
  pinMode(DEBUG_PIN, OUTPUT);

  // initialize the heartbeat LED and updating LED as output
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);

  // initialize settings
  update_settings(&settings);

  // ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino
  pinMode(readPin_adc_0, INPUT_DISABLE); // Not sure this does anything for us
  adc->adc0->setAveraging(4);   // set number of averages
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
  adc->adc0->startTimer(50000); // frequency in Hz - I don't think this is actually used since it's set later

  // end ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino

  // attach the external interrupt
  pinMode(EXTTRIG_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(EXTTRIG_PIN), ISR_exttrig, RISING);

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
    // Implement all updates
    update_settings(&settings);
    await_update = false;
  }
  
  if (abdma1.interrupted()) {
    digitalToggleFast(DEBUG_PIN);
    ProcessAnalogData(&abdma1, 0);
    adc->adc0->stopTimer();
    abdma1.clearCompletion();
    lock_adc = false;
  }

  if (ripe_trace) {
    encode_trace(); // Transfers Trace block into Trace-Proto block, and also clears ripe_trace and lock_trace flags.
    // TODO: Attempt to transfer data across the network
  }

  SCPI_Arduino_Loop_Update(); // process SCPI queries and commands
}

// ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino (with some modifications now)
void ProcessAnalogData(AnalogBufferDMA *pabdma, int8_t adc_num) {
  volatile uint16_t *pbuffer = pabdma->bufferLastISRFilled();
  //Serial.println(pabdma->bufferCountLastISRFilled());

  if ((uint32_t)pbuffer >= 0x20200000u)
    arm_dcache_delete((void *)pbuffer, sizeof(dma_adc_buff1));
  
  // Copy the buffer data into the image buffer
  if (!lock_trace) {
    lock_trace = true;
    trigcnt_trace = trigcnt_adc;
    int adcval;

    for (int i = 0; i < TRACE_NT; i++) {
      // POLARITY IS SWAPPED (assuming 12-bit ADC value) due for SIDEKICK MODEL 4
      adcval = *pbuffer;
      imgbuf_trace[i] = -(adcval - 1023); 
      pbuffer++;
    }
    pabdma->clearInterrupt();
    ripe_trace = true; // indicate that Trace block is filled and ready for usage
  }
}
// END ADC SETUP FROM EXAMPLE adc_timer_dma_oneshot.ino

// Works with the global variable imgbuf_trace and TRACE_NT
/*bool encode_imgbuf_trace(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
{
    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, &imgbuf_trace, TRACE_NT*sizeof(uint16_t));
} */

// Fills in structured data of an already-created data_t protobuffer (data trace protobuffer)
void update_data_t(dolphindaq_diode_Data* data_ptr, dolphindaq_diode_Trace* trace_ptr) {
  data_ptr->has_trace = true;
  data_ptr->trace = *trace_ptr;
  data_ptr->trace.has_shot_num = true;
  data_ptr->trace.shot_num = trigcnt_trace;
  //data_ptr->trace.yvals.funcs.encode = &encode_imgbuf_trace;
  //data_ptr->trace.has_shot_time = true;
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


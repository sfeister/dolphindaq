// Elements copied from https://arduino.stackexchange.com/questions/69509/how-can-i-make-stdcout-write-to-serial
// Elements about Serial read copied from https://www.arduino.cc/reference/en/language/functions/communication/serial/read/

#include <Arduino.h>
#include "scpi-def.h"

char incoming_byte = 0; // for incoming serial data

size_t SCPI_Write(scpi_t * context, const char * data, size_t len) {
    (void) context;
    Serial.write(data, len);
    return len;
}

scpi_result_t SCPI_Flush(scpi_t * context) {
    (void) context;
    Serial.flush();
    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t * context, int_fast16_t err) {
    (void) context;
    Serial.print("**ERROR: ");
    Serial.print(err);
    Serial.print(", \"");
    Serial.print(SCPI_ErrorTranslate(err));
    Serial.println("\"");
    return 0;
}

scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
    (void) context;
//  TODO: Make this print correctly
    if (SCPI_CTRL_SRQ == ctrl) {
        Serial.print("**SRQ: 0x");
        Serial.print(val, HEX);
        Serial.print("(");
        Serial.print(val, DEC);
        Serial.println(")");
    } else {
        Serial.print("**CTRL: ");
        Serial.print(val, HEX);
        Serial.print("(");
        Serial.print(val, DEC);
        Serial.println(")");
    }
    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t * context) {
    (void) context;
    // TODO: Reset the device as appropriate
    Serial.println("**Reset");
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SystemCommTcpipControlQ(scpi_t * context) {
    (void) context;
    // TODO: Handle as needed
    return SCPI_RES_ERR;
}

// the setup routine runs once when you press reset:
void setup() {
  // initialize the digital pin as an output.
  SCPI_Init(&scpi_context,
          scpi_commands,
          &scpi_interface,
          scpi_units_def,
          SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
          scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
          scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);

  Serial.begin(9600);
  while (!Serial); // wait for serial to finish initializing
  Serial.println("SCPI Interactive demo");
}

// the loop routine runs over and over again forever:
void loop() {
  // send data only when you receive data:
  if (Serial.available() > 0) {
      incoming_byte = Serial.read();
      SCPI_Input(&scpi_context, &incoming_byte, 1);
  }
}

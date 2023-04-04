/*

Code for serial-based Arduino communication for SCPI Parser

*/

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
    Serial.println("**Reset");
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SystemCommTcpipControlQ(scpi_t * context) {
    (void) context;
    return SCPI_RES_ERR;
}

// Call this in the "setup()" element of the .ino file
void SCPI_Arduino_Setup() {
  SCPI_Init(&scpi_context,
          scpi_commands,
          &scpi_interface,
          scpi_units_def,
          SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
          scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
          scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);

  Serial.begin(9600);
  while (!Serial); // wait for serial to finish initializing
}

// Call this each iteration of the "loop()" in the .ino file
void SCPI_Arduino_Loop_Update(){
    // send data to SCPI parser only when you receive data:
  if (Serial.available() > 0) {
      incoming_byte = Serial.read();
      SCPI_Input(&scpi_context, &incoming_byte, 1);
  }
}

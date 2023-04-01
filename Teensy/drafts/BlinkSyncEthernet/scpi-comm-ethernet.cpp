/*

Code for ethernet-based Arduino communication for SCPI Parser

Copying code from: https://github.com/ssilverman/QNEthernet/blob/master/examples/LengthWidthServer/LengthWidthServer.ino

*/

#include <Arduino.h>
#include <cstdio>
#include <QNEthernet.h>
#include "scpi-def.h"

using namespace qindesign::network;

// --------------------------------------------------------------------------
//  Configuration
// --------------------------------------------------------------------------

constexpr uint32_t kDHCPTimeout = 15000;  // 15 seconds
constexpr uint16_t kServerPort = 5000;


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


EthernetServer server{kServerPort};

// Call this in the "setup()" element of the .ino file
void SCPI_Arduino_Setup() {
  SCPI_Init(&scpi_context,
          scpi_commands,
          &scpi_interface,
          scpi_units_def,
          SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
          scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
          scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);

  // Code below copied from: https://github.com/ssilverman/QNEthernet/blob/master/examples/LengthWidthServer/LengthWidthServer.ino
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial to initialize
  }
  stdPrint = &Serial;  // Make printf work (a QNEthernet feature)
  printf("Starting...\r\n");
  // Unlike the Arduino API (which you can still use), QNEthernet uses
  // the Teensy's internal MAC address by default, so we can retrieve
  // it here
  uint8_t mac[6];
  Ethernet.macAddress(mac);  // This is informative; it retrieves, not sets
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  printf("Starting Ethernet with DHCP...\r\n");
  if (!Ethernet.begin()) {
    printf("Failed to start Ethernet\r\n");
    return;
  }
  if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
    printf("Failed to get IP address from DHCP\r\n");
  } else {
    IPAddress ip = Ethernet.localIP();
    printf("    Local IP    = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
    ip = Ethernet.subnetMask();
    printf("    Subnet mask = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
    ip = Ethernet.gatewayIP();
    printf("    Gateway     = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
    ip = Ethernet.dnsServerIP();
    printf("    DNS         = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);

    // Start the server
    printf("Listening for clients on port %u...\r\n", kServerPort);
    server.begin();
  }
}

// Call this each iteration of the "loop()" in the .ino file
void SCPI_Arduino_Loop_Update(){
    // send data to SCPI parser only when you receive data:
  if (Serial.available() > 0) {
      incoming_byte = Serial.read();
      SCPI_Input(&scpi_context, &incoming_byte, 1);
  }
}

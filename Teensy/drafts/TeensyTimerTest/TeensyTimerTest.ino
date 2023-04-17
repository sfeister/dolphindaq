#include "TeensyTimerTool.h"
using namespace TeensyTimerTool;

OneShotTimer t1; 
uint32_t reload;

void ch1_callback()
{
    Serial.println("Callback called.");
}

void setup() {
  Serial.begin(9600);
  while (!Serial); // wait for serial to finish initializing
  Serial.println("Beginnning...");
  t1.begin(ch1_callback);
}

void loop() {
  delay(500);
  Serial.println("Tick...");
  t1.trigger(10000);
  t1.getTriggerReload(0, &reload);
  Serial.print("T1: ");
  Serial.println(t1);
  delay(500);
  Serial.println("Tock...");
  t1.getTriggerReload(0, &reload);
  Serial.print("T1: ");
  Serial.println(t1);
}

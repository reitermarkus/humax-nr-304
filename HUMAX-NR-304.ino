#include <Scheduler.h>

#include "network.hpp"
#include "server.hpp"

#ifdef ARDUINO_ESP8266_NODEMCU
  const uint16_t IR_SEND_PIN = D2;
  const uint16_t IR_RECV_PIN = D5;
#else
  const uint16_t IR_SEND_PIN = 4;
  const uint16_t IR_RECV_PIN = 13;
#endif

NetworkTask network_task;
ServerTask server_task(IR_SEND_PIN, IR_RECV_PIN);

void setup() {
  Serial.begin(115200);

  Scheduler.start(&network_task);
  Scheduler.start(&server_task);
  Scheduler.begin();
}

void loop() { }

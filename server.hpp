#pragma once

#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

// The following are only needed for extended decoding of A/C Messages
#include <ir_Coolix.h>
#include <ir_Daikin.h>
#include <ir_Fujitsu.h>
#include <ir_Gree.h>
#include <ir_Haier.h>
#include <ir_Hitachi.h>
#include <ir_Kelvinator.h>
#include <ir_Midea.h>
#include <ir_Mitsubishi.h>
#include <ir_Panasonic.h>
#include <ir_Samsung.h>
#include <ir_Toshiba.h>
#include <ir_Whirlpool.h>

// Display the human readable state of an A/C message if we can.
void dumpACInfo(decode_results *results) {
  String description = "";

  #if DECODE_DAIKIN
    if (results->decode_type == DAIKIN) {
      IRDaikinESP ac(0);
      ac.setRaw(results->state);
      description = ac.toString();
    }
  #endif  // DECODE_DAIKIN

  #if DECODE_FUJITSU_AC
    if (results->decode_type == FUJITSU_AC) {
      IRFujitsuAC ac(0);
      ac.setRaw(results->state, results->bits / 8);
      description = ac.toString();
    }
  #endif  // DECODE_FUJITSU_AC

  #if DECODE_KELVINATOR
    if (results->decode_type == KELVINATOR) {
      IRKelvinatorAC ac(0);
      ac.setRaw(results->state);
      description = ac.toString();
    }
  #endif  // DECODE_KELVINATOR

  #if DECODE_MITSUBISHI_AC
    if (results->decode_type == MITSUBISHI_AC) {
      IRMitsubishiAC ac(0);
      ac.setRaw(results->state);
      description = ac.toString();
    }
  #endif  // DECODE_MITSUBISHI_AC

  #if DECODE_TOSHIBA_AC
    if (results->decode_type == TOSHIBA_AC) {
      IRToshibaAC ac(0);
      ac.setRaw(results->state);
      description = ac.toString();
    }
  #endif  // DECODE_TOSHIBA_AC

  #if DECODE_GREE
    if (results->decode_type == GREE) {
      IRGreeAC ac(0);
      ac.setRaw(results->state);
      description = ac.toString();
    }
  #endif  // DECODE_GREE

  #if DECODE_MIDEA
    if (results->decode_type == MIDEA) {
      IRMideaAC ac(0);
      ac.setRaw(results->value);  // Midea uses value instead of state.
      description = ac.toString();
    }
  #endif  // DECODE_MIDEA

  #if DECODE_HAIER_AC
    if (results->decode_type == HAIER_AC) {
      IRHaierAC ac(0);
      ac.setRaw(results->state);
      description = ac.toString();
    }
  #endif  // DECODE_HAIER_AC

  #if DECODE_HAIER_AC_YRW02
    if (results->decode_type == HAIER_AC_YRW02) {
      IRHaierACYRW02 ac(0);
      ac.setRaw(results->state);
      description = ac.toString();
    }
  #endif  // DECODE_HAIER_AC_YRW02

  #if DECODE_SAMSUNG_AC
    if (results->decode_type == SAMSUNG_AC) {
      IRSamsungAc ac(0);
      ac.setRaw(results->state);
      description = ac.toString();
    }
  #endif  // DECODE_SAMSUNG_AC

  #if DECODE_COOLIX
    if (results->decode_type == COOLIX) {
      IRCoolixAC ac(0);
      ac.setRaw(results->value);  // Coolix uses value instead of state.
      description = ac.toString();
    }
  #endif  // DECODE_COOLIX

  #if DECODE_PANASONIC_AC
    if (results->decode_type == PANASONIC_AC &&
        results->bits > kPanasonicAcShortBits) {
      IRPanasonicAc ac(0);
      ac.setRaw(results->state);
      description = ac.toString();
    }
  #endif  // DECODE_PANASONIC_AC

  #if DECODE_HITACHI_AC
    if (results->decode_type == HITACHI_AC) {
      IRHitachiAc ac(0);
      ac.setRaw(results->state);
      description = ac.toString();
    }
  #endif  // DECODE_HITACHI_AC

  #if DECODE_WHIRLPOOL_AC
    if (results->decode_type == WHIRLPOOL_AC) {
      IRWhirlpoolAc ac(0);
      ac.setRaw(results->state);
      description = ac.toString();
    }
  #endif  // DECODE_WHIRLPOOL_AC

  // If we got a human-readable description of the message, display it.
  if (description != "") Serial.println("Mesg Desc.: " + description);
}

class ServerTask: public Task {
  public:
    ServerTask(uint16_t send_pin, uint16_t recv_pin): Task(), server(80), irsend(send_pin), irrecv(recv_pin, recv_buffer_size, recv_timeout, true) {}

  private:
    ESP8266WebServer server;
    IRsend irsend;

    #if DECODE_AC
    // Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
    // A value this large may swallow repeats of some protocols
      const uint8_t recv_timeout = 50;
    #else
    // Suits most messages, while not swallowing many repeats.
      const uint8_t recv_timeout = 15;
    #endif

    const uint16_t recv_buffer_size = 1024;
    const uint16_t recv_min_unknown_size = 12;

    IRrecv irrecv;

  protected:
    void setup() {
      Serial.println("Enabling IR LED ...");
      irsend.begin();

      Serial.println("Enabling IR receiver ...");
      #if DECODE_HASH
        // Ignore messages with less than minimum on or off pulses.
        irrecv.setUnknownThreshold(recv_min_unknown_size);
      #endif
      irrecv.enableIRIn();

      Serial.println("Starting web server ...");

      server.on("/", HTTP_POST, [this]() {
        String json = server.arg("plain");

        DynamicJsonDocument doc(1024);

        DeserializationError error = deserializeJson(doc, json);

        if (error) {
          server.send(400, "application/json", "{\"error\":\"invalid JSON\"}");
          return;
        }

        const JsonObject& obj = doc.as<JsonObject>();

        if (!obj.containsKey("data")) {
          server.send(400, "application/json", "{\"error\":\"missing key \\\"data\\\"\"}");
          return;
        }

        int num = obj["data"];

        Serial.println(num);

        server.send(200, "application/json", json);
      });

      server.on("/", HTTP_ANY, [this]() {
        server.send(200, "application/json", "\"It works!\"");
      });

      server.on("/irlearn", HTTP_GET, [this]() {
        decode_results results;

        unsigned long start = millis();

        Serial.println("Learning IR code ...");

        while (start + 10000 > millis()) {
          if (irrecv.decode(&results)) {
            if (results.overflow) {
              server.send(500, "application/json", "{\"error\":\"IR code too big for buffer.\"}");
              return;
            }

            Serial.print(resultToHumanReadableBasic(&results));

            StaticJsonDocument<2048> doc;
            JsonObject obj = doc.to<JsonObject>();

            obj["type"]        = typeToString(results.decode_type, results.repeat);
            obj["code"]        = resultToHexidecimal(&results);

            obj["timing_info"] = resultToTimingInfo(&results);
            obj["source_code"] = resultToSourceCode(&results);

            String json = "";
            serializeJson(obj, json);
            server.send(200, "application/json", json);
            return;
          }

          yield();
        }

        server.send(504, "application/json", "{\"error\":\"Timeout while trying to receive IR signal.\"}");
      });

      server.on("/sendir", HTTP_POST, [this]() {
        String json = server.arg("plain");

        DynamicJsonDocument doc(1024);

        DeserializationError error = deserializeJson(doc, json);

        if (error) {
          server.send(400, "application/json", "{\"error\":\"invalid JSON\"}");
          return;
        }

        const JsonObject& obj = doc.as<JsonObject>();

        if (!obj.containsKey("data")) {
          server.send(400, "application/json", "{\"error\":\"missing key \\\"data\\\"\"}");
          return;
        }

        if (!obj.containsKey("bits")) {
          server.send(400, "application/json", "{\"error\":\"missing key \\\"bits\\\"\"}");
          return;
        }

        if (!obj.containsKey("type")) {
          server.send(400, "application/json", "{\"error\":\"missing key \\\"type\\\"\"}");
          return;
        }

        String type = obj["type"];

        if (type == "NEC") {
          uint64_t data = obj["data"];
          uint16_t bits = obj["bits"];

          Serial.printf("Sending IR code (%d bits): 0x", bits);
          serialPrintUint64(data, HEX);
          Serial.println();
          irsend.sendNEC(data, bits);
          server.send(204, "application/json", "");
          return;
        }

        server.send(400, "application/json", "{\"error\":\"invalid type \\\"" + type + "\\\"\"}");
      });

      server.begin();
    }

    void loop() {
      server.handleClient();
      yield();
    }
};

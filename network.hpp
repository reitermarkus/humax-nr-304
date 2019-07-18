#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiManager.h>

class NetworkTask: public Task {
  private:
    static const int HOSTNAME_ADDRESS = 0;
    static const int HOSTNAME_MAX_LEN = 24;

    char custom_hostname[HOSTNAME_MAX_LEN + 1];

    static bool shouldSaveConfig;

    static void saveConfig() {
      shouldSaveConfig = true;
    }

  protected:
    void setup() {
      WiFiManager wifiManager;

      EEPROM.begin(sizeof(custom_hostname));

      EEPROM.get(HOSTNAME_ADDRESS, custom_hostname);

      if (strlen(custom_hostname) > 0 && strlen(custom_hostname) <= HOSTNAME_MAX_LEN) {
        Serial.printf("Using saved hostname “%s”.\n", custom_hostname);
        WiFi.hostname(custom_hostname);
      } else {
        strcpy(custom_hostname, "");
      }

      WiFiManagerParameter hostname_parameter("hostname", "hostname", custom_hostname, HOSTNAME_MAX_LEN);
      wifiManager.addParameter(&hostname_parameter);

      wifiManager.setSaveConfigCallback(NetworkTask::saveConfig);

      wifiManager.setConfigPortalTimeout(3 * 60);

      if (!wifiManager.autoConnect()) {
        Serial.println("Failed to connect and hit timeout, resetting ...");
        ESP.reset();
        return;
      }

      while (true) {
        if (NetworkTask::shouldSaveConfig) {
          if (WiFi.hostname(hostname_parameter.getValue())) {
            memset(custom_hostname, 0, sizeof(custom_hostname));
            strcpy(custom_hostname, hostname_parameter.getValue());

            Serial.printf("Saving hostname “%s” …\n", custom_hostname);
            EEPROM.put(HOSTNAME_ADDRESS, custom_hostname);
            EEPROM.commit();
          } else {
            Serial.printf("Hostname “%s” is invalid.\n", hostname_parameter.getValue());
          }
        }

        if (strlen(custom_hostname) > 0) {
          break;
        }

        Serial.printf("No hostname set, restarting config portal ...\n");

        NetworkTask::shouldSaveConfig = false;

        if (!wifiManager.startConfigPortal()) {
          ESP.reset();
          return;
        }
      }

      Serial.printf("Connected to “%s” with IP “%s”.\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

      if (MDNS.begin(custom_hostname)) {
        Serial.printf("MDNS responder started with hostname “%s”.\n", custom_hostname);
      } else {
        Serial.println("Failed to start MDNS responder, resetting …");
        ESP.reset();
        return;
      }
    }

    void loop() {
      MDNS.update();
      yield();
    }
};

bool NetworkTask::shouldSaveConfig = false;

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "";
const char* password = "";

#include <Bluepad32.h>
#include "BTLEController.h"

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

BTLEController BC;
long oldUpdate = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("o/");

  /* ------------ WIFI SETUP ------------ */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  /* ----------- OTA ----------- */
  ArduinoOTA.setHostname("XIAO C3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Set the global pointer to the BTLEController instance
  BTLEController::controllerInstance = &BC;
  BC.setup();

  pwm.begin();
  pwm.setPWMFreq(1000);
  Wire.setClock(400000);

  BC.getSavedController();
}

long delayTime = 20;
void loop() {
  BP32.update();
  ArduinoOTA.handle();
  
  if (millis() - oldUpdate > delayTime) {

    if (BC.isConnected) {
      int axisX = BC.myGamepad->axisX();
      Serial.println(axisX);

      if (axisX > 20) {
        pwm.setPWM(15, 4096 - axisX * 8, 0);
        pwm.setPWM(13, 4096 - axisX * 8, 0);
        pwm.setPWM(11, 4096 - axisX * 8, 0);
        pwm.setPWM(9, 4096 - axisX * 8, 0);
      }

      if (axisX >= -20 && axisX <= 20) {
        pwm.setPWM(15, 0, 4096);
        pwm.setPWM(14, 0, 4096);
        pwm.setPWM(13, 0, 4096);
        pwm.setPWM(12, 0, 4096);
        pwm.setPWM(11, 0, 4096);
        pwm.setPWM(10, 0, 4096);
        pwm.setPWM(9, 0, 4096);
        pwm.setPWM(8, 0, 4096);
      }

      if (axisX < -20) {
        pwm.setPWM(14, 4102-abs(axisX) * 8, 0);
        pwm.setPWM(12, 4102-abs(axisX) * 8, 0);
        pwm.setPWM(10, 4102-abs(axisX) * 8, 0);
        pwm.setPWM(8, 4102-abs(axisX) * 8, 0);
      }
    }

    oldUpdate = millis();
  }

  if (Serial.available() > 0) {
    String msg = Serial.readStringUntil('\n');
    Serial.println("GOT> " + msg);

    if (msg.indexOf("SAVE") > -1) {
      Serial.println("GOT CMD - SAVE");
      if (BC.myGamepad != nullptr) {
        BC.saveCurrentController();
        BC.getSavedController();

      } else {
        Serial.println("No gamepad!");
      }
    }

    if (msg.indexOf("PRINT") > -1) {
      Serial.println("GOT CMD - PRINT");
      BC.getSavedController();
    }
  }
}

#ifndef SERVER_SETUP_
#define SERVER_SETUP_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <Helpers.h>
#include <Preferences.h>
#include <Sensors.h>
#include <Update.h>
#include <index.h>

extern SensorRelayManager manager;
extern JsonDocument doc;
extern InternalTime internal_time;
extern const char *localUrl;

void SetupServer(AsyncWebServer &server, const IPAddress &localIP) {
  server.on("/wpad.dat",
            [](AsyncWebServerRequest *request) { request->send(404); });
  server.on("/connecttest.txt", [](AsyncWebServerRequest *request) {
    request->redirect("http://logout.net");
  });
  server.on("/redirect", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  });
  server.on("/generate_204", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  });
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  });
  server.on("/ncsi.txt", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  });
  server.on("/success.txt",
            [](AsyncWebServerRequest *request) { request->send(200); });
  server.on("/canonical.html", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  });
  server.on("/favicon.ico",
            [](AsyncWebServerRequest *request) { request->send(404); });

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response =
        request->beginResponse(200, "text/html", MAIN_page);
    response->addHeader("Cache-Control", "public,max-age=31536000");
    request->send(response);
  });

  server.on("/SendForms", [](AsyncWebServerRequest *request) {
    String response = request->getParam(0)->value();
    request->send(200, "text/plain", "All good");
    deserializeJson(doc, response);
    int minute = doc["Minute"];
    int hour = doc["Hour"];
    internal_time.SetEpoch((Wrap(minute, 0, 59) * 60) +
                           (Wrap(hour, 0, 23) * 3600));
  });

  server.on("/submit-time", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam(0)) {
      int newEpoch = request->getParam(0)->value().toInt();
      if (newEpoch != 0) {
        internal_time.ResetRTC();
        internal_time.SetEpoch(newEpoch);

        request->send(
            200, "plaintext/text",
            ("successfully got time at " + request->getParam(0)->value()));
      } else {
        request->send(200, "plaintext/text", "Invalid time, try again");
      }
    } else {
      request->send(200, "plaintext/text", "Invalid time, try again");
    }
  });

  server.on("/readValues", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    JsonArray sensorsArray = doc["sensors"].to<JsonArray>(); // Updated
    JsonArray relaysArray = doc["relays"].to<JsonArray>();   // Updated

    for (int i = 0; i < manager.GetNumSensors(); i++) {
      Sensor *sensor = manager.sensors[i];
      if (sensor) {
        JsonObject sensorObj = sensorsArray.add<JsonObject>(); // Updated
        sensorObj["id"] = sensor->GetId();
        float value = sensor->GetValue();
        sensorObj["value"] = value;
      }
    }

    for (int i = 0; i < manager.GetNumRelays(); i++) {
      Relay *relay = manager.relays[i];
      if (relay) {
        JsonObject relayObj = relaysArray.add<JsonObject>(); // Updated
        relayObj["id"] = relay->GetId();
        bool status = relay->IsOn();
        relayObj["status"] = status;
      }
    }

    String jsonString;
    serializeJson(doc, jsonString);
    request->send(200, "application/json", jsonString);
  });

  server.on("/readADC", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    JsonArray sensorsArray = doc["sensors"].to<JsonArray>(); // Updated
    JsonArray relaysArray = doc["relays"].to<JsonArray>();   // Updated

    for (int i = 0; i < manager.GetNumSensors(); i++) {
      Sensor *sensor = manager.sensors[i];
      if (sensor) {
        JsonObject sensorObj = sensorsArray.add<JsonObject>(); // Updated
        sensorObj["id"] = sensor->GetId();
        sensorObj["pin"] = sensor->GetPin();
        sensorObj["name"] = sensor->GetName();
        float value = sensor->GetValue();
        sensorObj["value"] = value;
      }
    }

    for (int i = 0; i < manager.GetNumRelays(); i++) {
      Relay *relay = manager.relays[i];
      if (relay) {
        JsonObject relayObj = relaysArray.add<JsonObject>(); // Updated
        relayObj["id"] = relay->GetId();
        relayObj["name"] = relay->GetName();
        relayObj["pin"] = relay->GetPin();
        JsonArray conditionsArray =
            relayObj["conditions"].to<JsonArray>(); // Updated
        for (int j = 0; j < MAX_CONDITIONS; j++) {
          Condition *condition = relay->GetCondition(j);
          if (condition) {
            JsonObject condObj = conditionsArray.add<JsonObject>(); // Updated
            condObj["id"] = condition->GetId();
            condObj["sensor"] = condition->GetSensor();
            condObj["sensorId"] = condition->GetSensorId();
            condObj["operator"] = condition->GetOperator();
            condObj["value"] = condition->GetValue();
            condObj["type"] = condition->GetType();
          } else {
            break;
          }
        }
      }
    }
    String jsonString;
    serializeJson(doc, jsonString);
    request->send(200, "application/json", jsonString);
  });

  // server.on("/submit-wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
  //   // if (request->hasParam(0, true)) {
  //     Serial.println("got wifi");
  //   for (size_t i = 0; i < request->params(); i++) {
  //     Serial.println(i);
  //     Serial.println(request->getParam(i)->value());
  //   }
  //     Serial.println(request->getParam("ssid")->value());
  //     request->send(200, "text/plain", "Got wifi");
  //   // } else {
  //     request->send(200, "text/plain", "Failed to get wifi, blank request");
  //   // }
  // });

  server.on("/submit-wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    bool wifiMode = false;
    String ssid = "";
    String password = "";

    if (request->params() >= 3) {
      Serial.println("got wifi");
      for (size_t i = 0; i < request->params(); i++) {
        Serial.println(i);
        Serial.println(request->getParam(i)->value());
      }

      // Wifi mode (wifi/hotspot)
      AsyncWebParameter *modeParam = request->getParam(0);
      if (modeParam != nullptr) {
        wifiMode = (modeParam->value() == "hotspot") ? 0 : 1;
      }

      // Ssid
      AsyncWebParameter *ssidParam = request->getParam(1);
      if (ssidParam != nullptr) {
        ssid = ssidParam->value();
      }

      // Password
      AsyncWebParameter *passParam = request->getParam(2);
      if (passParam != nullptr) {
        password = passParam->value();
      }

      Preferences prefs;
      prefs.begin("wifi", false);
      Serial.println("Wifi mode: " + wifiMode);
      prefs.putBool("wifimode", wifiMode);
      prefs.putString("wifissid", ssid);
      prefs.putString("wifipass", password);
      prefs.end();

      request->send(200, "text/plain", "Got wifi");
    } else {
      request->send(200, "text/plain", "Failed to get wifi, blank request");
    }
  });

  server.on(
      "/update", HTTP_POST,
      [](AsyncWebServerRequest *request) {
        if (Update.hasError()) {
          request->send(500, "text/plain", "Update failed");
        } else {
          request->send(200, "text/plain", "Update successful. Rebooting...");
          delay(1000);
          ESP.restart();
        }
      },
      [](AsyncWebServerRequest *request, String filename, size_t index,
         uint8_t *data, size_t len, bool final) {
        if (!index) {
          if (!Update.begin(UPDATE_SIZE_UNKNOWN))
            return;
        }
        if (len) {
          Update.write(data, len);
        }
        if (final) {
          Update.end(true);
        }
      });

  server.on("/submit-sensors", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("body", true)) {
      String jsonStr = request->getParam("body", true)->value();
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, jsonStr);

      if (error) {
        Serial.println("Invalid JSON received: " + String(error.c_str()));
        request->send(400, "text/plain", "Invalid JSON");
        return;
      }

      manager.Clear();

      JsonArray sensorsArray = doc["sensors"];
      for (JsonObject sensorObj : sensorsArray) {
        uint8_t id = sensorObj["id"].as<int>();
        String name = sensorObj["name"].as<String>();
        uint8_t pin = sensorObj["pin"].as<int>();
        float value = 0.0;
        bool folded = true;
        Sensor *sensor = new Sensor(id, name.c_str(), pin, value, folded);
        manager.RegisterSensor(sensor);
      }

      JsonArray relaysArray = doc["relays"];
      for (JsonObject relayObj : relaysArray) {
        uint8_t id = relayObj["id"].as<int>();
        String name = relayObj["name"].as<String>();
        uint8_t pin = relayObj["pin"].as<int>();
        bool status = false;
        bool folded = false;
        Relay *relay = new Relay(id, name.c_str(), pin, status, folded);

        JsonArray conditionsArray = relayObj["conditions"];
        for (JsonObject conditionObj : conditionsArray) {
          uint8_t conditionId = conditionObj["id"].as<int>();
          uint8_t sensor = conditionObj["sensor"].as<int>();
          uint8_t sensorId = conditionObj["sensorId"].as<int>();
          String op = conditionObj["operator"].as<String>();
          float value = conditionObj["value"].as<float>();
          String type = conditionObj["type"].as<String>();
          Condition *condition = new Condition(
              sensor, sensorId, op.c_str(), value, conditionId, type.c_str());
          relay->AddCondition(condition);
        }

        manager.RegisterRelay(relay);
        pinMode(pin, OUTPUT);
        digitalWrite(pin, status ? HIGH : LOW);
      }

      manager.SaveToPreferences();
      request->send(200, "text/plain", "Sensors and relays updated");
    } else {
      Serial.println("No data received in request");
      request->send(400, "text/plain", "No data received");
    }
  });
}

#endif // SERVER_SETUP_H

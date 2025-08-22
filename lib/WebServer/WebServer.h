#ifndef SERVER_SETUP_H
#define SERVER_SETUP_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <index.h>
#include <Sensors.h>
#include <Update.h>
#include <Helpers.h>

extern SensorRelayManager manager;
extern JsonDocument doc;
extern InternalTime internal_time;
extern const char* localUrl;

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

  server.on("/readValues", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    JsonArray sensorsArray = doc.createNestedArray("sensors");
    JsonArray relaysArray = doc.createNestedArray("relays");

    for (int i = 0; i < manager.GetNumSensors(); i++) {
      Sensor *sensor = manager.sensors[i];
      if (sensor) {
        JsonDocument sensorDoc;
        sensorDoc["id"] = sensor->GetId();
        float value = sensor->GetValue();
        sensorDoc["value"] = value;
        sensorsArray.add(sensorDoc);
      }
    }

    for (int i = 0; i < manager.GetNumRelays(); i++) {
      Relay *relay = manager.relays[i];
      if (relay) {
        JsonDocument relayDoc;
        relayDoc["id"] = relay->GetId();
        bool status = relay->IsOn();
        relayDoc["status"] = status;
        relaysArray.add(relayDoc);
      }
    }

    String jsonString;
    serializeJson(doc, jsonString);
    request->send(200, "application/json", jsonString);
  });

  server.on("/readADC", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    JsonArray sensorsArray = doc.createNestedArray("sensors");
    JsonArray relaysArray = doc.createNestedArray("relays");

    for (int i = 0; i < manager.GetNumSensors(); i++) {
      Sensor *sensor = manager.sensors[i];
      if (sensor) {
        JsonDocument sensorDoc;
        sensorDoc["id"] = sensor->GetId();
        sensorDoc["pin"] = sensor->GetPin();
        sensorDoc["name"] = sensor->GetName();
        float value = sensor->GetValue();
        sensorDoc["value"] = value;
        sensorsArray.add(sensorDoc);
      }
    }

    for (int i = 0; i < manager.GetNumRelays(); i++) {
      Relay *relay = manager.relays[i];
      if (relay) {
        JsonDocument relayDoc;
        relayDoc["id"] = relay->GetId();
        relayDoc["name"] = relay->GetName();
        relayDoc["pin"] = relay->GetPin();
        JsonArray conditionsArray = relayDoc.createNestedArray("conditions");
        for (int j = 0; j < MAX_CONDITIONS; j++) {
          Condition *condition = relay->GetCondition(j);
          if (condition) {
            JsonDocument condDoc;
            condDoc["id"] = condition->GetId();
            condDoc["sensor"] = condition->GetSensor();
            condDoc["sensorId"] = condition->GetSensorId();
            condDoc["operator"] = condition->GetOperator();
            condDoc["value"] = condition->GetValue();
            condDoc["type"] = condition->GetType();
            conditionsArray.add(condDoc);
          } else {
            break;
          }
        }
        relaysArray.add(relayDoc);
      }
    }
    String jsonString;
    serializeJson(doc, jsonString);
    request->send(200, "application/json", jsonString);
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
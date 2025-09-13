#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Preferences.h> // Ensure Preferences.h is included for SaveToPreferences
#include <ArduinoJson.h> // Ensure ArduinoJson.h is included

/**
 * @brief Maximum number of sensors, relays, and conditions
 */
#define MAX_SENSORS 10
#define MAX_RELAYS 10
#define MAX_CONDITIONS 5

/**
 * @brief Class representing a condition for a relay
 */
class Condition {
public:
  Condition(uint8_t sensor, uint8_t sensorId, const char *op, float value,
            uint8_t id, const char *type)
      : sensor(sensor), sensorId(sensorId), operator_(op ? op : ""),
        value(value), id(id), type_(type ? type : "") {}

  uint8_t GetSensor() const { return sensor; }
  uint8_t GetSensorId() const { return sensorId; }
  const char *GetOperator() const { return operator_.c_str(); }
  float GetValue() const { return value; }
  uint8_t GetId() const { return id; }
  const char *GetType() const { return type_.c_str(); }

private:
  uint8_t sensor;
  uint8_t sensorId;
  String operator_;
  float value;
  uint8_t id;
  String type_;
};

/**
 * @brief Class representing a sensor
 */
class Sensor {
public:
  Sensor(uint8_t id, const char *name, uint8_t pin, float value,
         bool folded = true)
      : id(id), name(name ? name : ""), pin(pin), value(value), folded(folded) {}

  void SetValue(float newValue) { value = newValue; }
  uint8_t GetId() const { return id; }
  const char *GetName() const { return name.c_str(); }
  uint8_t GetPin() const { return pin; }
  float GetValue() const { return value; }
  bool GetFolded() const { return folded; }

private:
  uint8_t id;
  String name;
  uint8_t pin;
  float value;
  bool folded;
};

/**
 * @brief Class representing a relay
 */
class Relay {
public:
  Relay(uint8_t id, const char *name, uint8_t pin, bool status = false,
        bool folded = false)
      : id(id), name(name ? name : ""), pin(pin), status(status),
        folded(folded) {
    for (int i = 0; i < MAX_CONDITIONS; i++) {
      conditions[i] = nullptr;
    }
  }

  void AddCondition(Condition *condition) {
    for (int i = 0; i < MAX_CONDITIONS; i++) {
      if (conditions[i] == nullptr) {
        conditions[i] = condition;
        break;
      }
    }
  }

  void SetStatus(bool newStatus) { status = newStatus; }
  void ToggleStatus() { status = !status; }
  bool IsOn() const { return status; }
  void MoveCondition(uint8_t conditionId, const char *direction) {
    int currentIndex = -1;
    for (int i = 0; i < MAX_CONDITIONS; i++) {
      if (conditions[i] && conditions[i]->GetId() == conditionId) {
        currentIndex = i;
        break;
      }
    }
    if (currentIndex == -1)
      return;

    bool canMoveUp = (strcmp(direction, "up") == 0) && (currentIndex > 0) &&
                     (conditions[currentIndex - 1] != nullptr);
    bool canMoveDown = (strcmp(direction, "down") == 0) &&
                       (currentIndex < MAX_CONDITIONS - 1) &&
                       (conditions[currentIndex + 1] != nullptr);

    if (canMoveUp) {
      Condition *temp = conditions[currentIndex];
      conditions[currentIndex] = conditions[currentIndex - 1];
      conditions[currentIndex - 1] = temp;
    } else if (canMoveDown) {
      Condition *temp = conditions[currentIndex];
      conditions[currentIndex] = conditions[currentIndex + 1];
      conditions[currentIndex + 1] = temp;
    }
  }

  uint8_t GetId() const { return id; }
  const char *GetName() const { return name.c_str(); }
  uint8_t GetPin() const { return pin; }
  bool GetStatus() const { return status; }
  bool GetFolded() const { return folded; }
  Condition *GetCondition(uint8_t index) const {
    return (index < MAX_CONDITIONS) ? conditions[index] : nullptr;
  }

  Condition *conditions[MAX_CONDITIONS];
private:
  uint8_t id;
  String name;
  uint8_t pin;
  bool status;
  bool folded;
};

/**
 * @brief Class to manage sensors and relays
 */
class SensorRelayManager {
public:
    SensorRelayManager() : num_sensors(0), num_relays(0) {
        for (int i = 0; i < MAX_SENSORS; i++) {
            sensors[i] = nullptr;
        }
        for (int i = 0; i < MAX_RELAYS; i++) {
            relays[i] = nullptr;
        }
    }

    ~SensorRelayManager() {
        Clear();
    }

    void Clear() {
        for (int i = 0; i < MAX_SENSORS; i++) {
            delete sensors[i];
            sensors[i] = nullptr;
        }
        num_sensors = 0;

        for (int i = 0; i < MAX_RELAYS; i++) {
            if (relays[i]) {
                for (int j = 0; j < MAX_CONDITIONS; j++) {
                    delete relays[i]->GetCondition(j);
                }
                delete relays[i];
                relays[i] = nullptr;
            }
        }
        num_relays = 0;
    }

    void RegisterSensor(Sensor* sensor) {
        if (num_sensors < MAX_SENSORS) {
            sensors[num_sensors] = sensor;
            num_sensors++;
        }
    }

    void RegisterRelay(Relay* relay) {
        if (num_relays < MAX_RELAYS) {
            relays[num_relays] = relay;
            num_relays++;
        }
    }

    Sensor* GetSensorById(uint8_t id) const {
        for (int i = 0; i < num_sensors; i++) {
            if (sensors[i] && sensors[i]->GetId() == id) {
                return sensors[i];
            }
        }
        return nullptr;
    }

    Relay* GetRelayById(uint8_t id) const {
        for (int i = 0; i < num_relays; i++) {
            if (relays[i] && relays[i]->GetId() == id) {
                return relays[i];
            }
        }
        return nullptr;
    }

    Relay* GetRelayArray() const {
        return *relays;
    }

    Sensor* GetSensorArray() const {
        return *sensors;
    }

    uint8_t GetNumSensors() const { return num_sensors; }
    uint8_t GetNumRelays() const { return num_relays; }

    void SaveToPreferences();
    void LoadFromPreferences();

    Sensor* sensors[MAX_SENSORS];
    Relay* relays[MAX_RELAYS];

private:
    uint8_t num_sensors;
    uint8_t num_relays;
};

void SensorRelayManager::SaveToPreferences() {
  Preferences prefs;
  prefs.begin("sensor_relay", false);

 JsonDocument doc;

  JsonArray sensorsArray = doc["sensors"].to<JsonArray>(); // Updated
  for (int i = 0; i < num_sensors; i++) {
    if (sensors[i]) {
      JsonObject obj = sensorsArray.add<JsonObject>(); // Updated
      obj["id"] = sensors[i]->GetId();
      obj["name"] = sensors[i]->GetName();
      obj["pin"] = sensors[i]->GetPin();
      obj["folded"] = sensors[i]->GetFolded();
    }
  }

  JsonArray relaysArray = doc["relays"].to<JsonArray>(); // Updated
  for (int i = 0; i < num_relays; i++) {
    if (relays[i]) {
      JsonObject obj = relaysArray.add<JsonObject>(); // Updated
      obj["id"] = relays[i]->GetId();
      obj["name"] = relays[i]->GetName();
      obj["pin"] = relays[i]->GetPin();
      obj["folded"] = relays[i]->GetFolded();

      JsonArray conds = obj["conditions"].to<JsonArray>(); // Updated
      for (int j = 0; j < MAX_CONDITIONS; j++) {
        Condition *c = relays[i]->GetCondition(j);
        if (c) {
          JsonObject condObj = conds.add<JsonObject>(); // Updated
          condObj["sensor"] = c->GetSensor();
          condObj["sensorId"] = c->GetSensorId();
          condObj["operator"] = c->GetOperator();
          condObj["value"] = c->GetValue();
          condObj["id"] = c->GetId();
          condObj["type"] = c->GetType();
        } else {
          break;
        }
      }
    }
  }

  String jsonStr;
  serializeJson(doc, jsonStr);
  if (prefs.putString("config", jsonStr) == 0) {
    Serial.println("Failed to save config to Preferences");
  } else {
    Serial.println("Config saved successfully");
  }

  prefs.end();
}

void SensorRelayManager::LoadFromPreferences() {
  Preferences prefs;
  prefs.begin("sensor_relay", true);

  String jsonStr = prefs.getString("config", "");
  if (jsonStr.isEmpty()) {
    Serial.println("No saved config found");
    prefs.end();
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonStr);
  if (error) {
    Serial.print("Failed to parse saved JSON: ");
    Serial.println(error.c_str());
    prefs.end();
    return;
  }

  JsonArray sensorsArray = doc["sensors"];
  for (JsonObject obj : sensorsArray) {
    uint8_t id = obj["id"];
    const char *name = obj["name"];
    uint8_t pin = obj["pin"];
    bool folded = obj["folded"] | true;

    Sensor *sensor = new Sensor(id, name, pin, 0.0f, folded);
    RegisterSensor(sensor);
  }

  JsonArray relaysArray = doc["relays"];
  for (JsonObject obj : relaysArray) {
    uint8_t id = obj["id"];
    const char *name = obj["name"];
    uint8_t pin = obj["pin"];
    bool folded = obj["folded"] | false;

    Relay *relay = new Relay(id, name, pin, false, folded);
    JsonArray conditionsArray = obj["conditions"];
    for (JsonObject condObj : conditionsArray) {
      uint8_t sensor = condObj["sensor"];
      uint8_t sensorId = condObj["sensorId"];
      const char *op = condObj["operator"];
      float value = condObj["value"];
      uint8_t condId = condObj["id"];
      const char *type = condObj["type"];

      Condition *condition =
          new Condition(sensor, sensorId, op, value, condId, type);
      relay->AddCondition(condition);
    }
    RegisterRelay(relay);

    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  Serial.println("Config loaded successfully");
  prefs.end();
}

/**
 * @brief Reads the current value of a sensor
 * @param sensor Pointer to the Sensor object
 * @return float The current sensor value
 */
float readSensorValue(Sensor *sensor) {
  int rawValue = analogRead(sensor->GetPin());
  return rawValue * 0.0048828125;
}

/**
 * @brief Evaluates a single condition
 * @param op The operator (e.g., ">", "<")
 * @param sensorValue The current sensor value
 * @param conditionValue The condition threshold
 * @return bool True if the condition is met
 */
bool evaluateCondition(const char *op, float sensorValue,
                       float conditionValue) {
  if (strcmp(op, ">") == 0) {
    // Serial.println("Sensor Value: " + String(sensorValue));
    // Serial.println("Condition Threshold: " + String(conditionValue));
    return sensorValue > conditionValue;
  } else if (strcmp(op, "<") == 0) {
    // Serial.println("Sensor Value: " + String(sensorValue));
    // Serial.println("Condition Threshold: " + String(conditionValue));
    return sensorValue < conditionValue;
  } else if (strcmp(op, "=") == 0) {
    // Serial.println("Sensor Value: " + String(sensorValue));
    // Serial.println("Condition Threshold: " + String(conditionValue));
    return sensorValue == conditionValue;
  } else if (strcmp(op, ">=") == 0) {
    // Serial.println("Sensor Value: " + String(sensorValue));
    // Serial.println("Condition Threshold: " + String(conditionValue));
    return sensorValue >= conditionValue;
  } else if (strcmp(op, "<=") == 0) {
    // Serial.println("Sensor Value: " + String(sensorValue));
    // Serial.println("Condition Threshold: " + String(conditionValue));
    return sensorValue <= conditionValue;
  // } else if (strcmp(op, "!=") == 0) {
  //   Serial.println("Sensor Value: " + String(sensorValue));
  //   Serial.println("Condition Threshold: " + String(conditionValue));
    return sensorValue != conditionValue;
  } else {
    Serial.println("Invalid operator");
    return false;
  }
}

/**
 * @brief Finds a sensor by its ID
 * @param id The ID of the sensor
 * @return Sensor* Pointer to the sensor or nullptr if not found
 */
Sensor *findSensorById(uint8_t id, SensorRelayManager &manager) {
  return manager.GetSensorById(id);
}

/**
 * @brief Evaluates all conditions for a relay
 * @param relay The Relay object to evaluate
 * @param manager The SensorRelayManager containing sensor data
 * @return bool True if the relay should be on
 */
bool evaluateRelayConditions(Relay &relay, SensorRelayManager &manager) {
  bool hasConditions = false;
  bool allConditionsTrue = true;

  for (int j = 0; j < MAX_CONDITIONS; j++) {
    Condition *condition = relay.GetCondition(j);
    if (condition == nullptr)
      break;

    hasConditions = true;
    Sensor *sensor = findSensorById(condition->GetSensorId(), manager);

    if (sensor == nullptr) {
      allConditionsTrue = false;
      break;
    }

    float sensorValue = sensor->GetValue();
    float conditionValue = condition->GetValue();
    const char *op = condition->GetOperator();

    Serial.println(sensorValue);
    Serial.println(conditionValue);

    if (!evaluateCondition(op, sensorValue, conditionValue)) {
      allConditionsTrue = false;
      break;
    }
  }
  return hasConditions && allConditionsTrue;
}

#endif // SENSORS_H

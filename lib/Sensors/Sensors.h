#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

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
    Condition(uint8_t sensor, uint8_t sensorId, const char* op, float value, uint8_t id, const char* type)
        : sensor(sensor), sensorId(sensorId), operator_(op ? op : ""), value(value), id(id), type(type ? type : "") {}

    uint8_t GetSensor() const { return sensor; }
    uint8_t GetSensorId() const { return sensorId; }
    const char* GetOperator() const { return operator_; }
    float GetValue() const { return value; }
    uint8_t GetId() const { return id; }
    const char* GetType() const { return type; }

private:
    uint8_t sensor;
    uint8_t sensorId;
    const char* operator_;
    float value;
    uint8_t id;
    const char* type;
};

/**
 * @brief Class representing a sensor
 */
class Sensor {
public:
    Sensor(uint8_t id, const char* name, uint8_t pin, float value, bool folded = true)
        : id(id), name(name ? name : ""), pin(pin), value(value), folded(folded) {}

    void SetValue(float newValue) { value = newValue; }
    uint8_t GetId() const { return id; }
    const char* GetName() const { return name; }
    uint8_t GetPin() const { return pin; }
    float GetValue() const { return value; }
    bool GetFolded() const { return folded; }

private:
    uint8_t id;
    const char* name;
    uint8_t pin;
    float value;
    bool folded;
};

/**
 * @brief Class representing a relay
 */
class Relay {
public:
    Relay(uint8_t id, const char* name, uint8_t pin, bool status = false, bool folded = false)
        : id(id), name(name ? name : ""), pin(pin), status(status), folded(folded) {
        for (int i = 0; i < MAX_CONDITIONS; i++) {
            conditions[i] = nullptr;
        }
    }

    void AddCondition(Condition* condition) {
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
    void MoveCondition(uint8_t conditionId, const char* direction) {
        int currentIndex = -1;
        for (int i = 0; i < MAX_CONDITIONS; i++) {
            if (conditions[i] && conditions[i]->GetId() == conditionId) {
                currentIndex = i;
                break;
            }
        }
        if (currentIndex == -1) return;

        bool canMoveUp = (strcmp(direction, "up") == 0) && (currentIndex > 0) && (conditions[currentIndex - 1] != nullptr);
        bool canMoveDown = (strcmp(direction, "down") == 0) && (currentIndex < MAX_CONDITIONS - 1) && (conditions[currentIndex + 1] != nullptr);

        if (canMoveUp) {
            Condition* temp = conditions[currentIndex];
            conditions[currentIndex] = conditions[currentIndex - 1];
            conditions[currentIndex - 1] = temp;
        } else if (canMoveDown) {
            Condition* temp = conditions[currentIndex];
            conditions[currentIndex] = conditions[currentIndex + 1];
            conditions[currentIndex + 1] = temp;
        }
    }

    uint8_t GetId() const { return id; }
    const char* GetName() const { return name; }
    uint8_t GetPin() const { return pin; }
    bool GetStatus() const { return status; }
    bool GetFolded() const { return folded; }
    Condition* GetCondition(uint8_t index) const {
        return (index < MAX_CONDITIONS) ? conditions[index] : nullptr;
    }

private:
    uint8_t id;
    const char* name;
    uint8_t pin;
    bool status;
    bool folded;
    Condition* conditions[MAX_CONDITIONS];
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

private:
    Sensor* sensors[MAX_SENSORS];
    Relay* relays[MAX_RELAYS];
    uint8_t num_sensors;
    uint8_t num_relays;
};

/**
 * @brief Reads the current value of a sensor
 * @param sensor Pointer to the Sensor object
 * @return float The current sensor value
 */
float readSensorValue(Sensor* sensor) {
    int rawValue = analogRead(sensor->GetPin());
    return rawValue * 0.0048828125; // Example scaling: 0-1023 to 0-5V
}

/**
 * @brief Evaluates a single condition
 * @param op The operator (e.g., ">", "<")
 * @param sensorValue The current sensor value
 * @param conditionValue The condition threshold
 * @return bool True if the condition is met
 */
bool evaluateCondition(const char* op, float sensorValue, float conditionValue) {
    if (strcmp(op, ">") == 0) return sensorValue > conditionValue;
    else if (strcmp(op, "<") == 0) return sensorValue < conditionValue;
    else if (strcmp(op, "==") == 0) return sensorValue == conditionValue;
    else if (strcmp(op, ">=") == 0) return sensorValue >= conditionValue;
    else if (strcmp(op, "<=") == 0) return sensorValue <= conditionValue;
    else if (strcmp(op, "!=") == 0) return sensorValue != conditionValue;
    else return false;
}

/**
 * @brief Finds a sensor by its ID
 * @param id The ID of the sensor
 * @return Sensor* Pointer to the sensor or nullptr if not found
 */
Sensor* findSensorById(uint8_t id, SensorRelayManager& manager) {
    return manager.GetSensorById(id);
}

/**
 * @brief Evaluates all conditions for a relay
 * @param relay The Relay object to evaluate
 * @param manager The SensorRelayManager containing sensor data
 * @return bool True if the relay should be on
 */
bool evaluateRelayConditions(Relay& relay, SensorRelayManager& manager) {
    bool hasConditions = false;
    bool allConditionsTrue = true;

    for (int j = 0; j < MAX_CONDITIONS; j++) {
        Condition* condition = relay.GetCondition(j);
        if (condition == nullptr) break;

        hasConditions = true;
        Sensor* sensor = findSensorById(condition->GetSensorId(), manager);

        if (sensor == nullptr) {
            allConditionsTrue = false;
            break;
        }

        float sensorValue = sensor->GetValue();
        float conditionValue = condition->GetValue();
        const char* op = condition->GetOperator();

        if (!evaluateCondition(op, sensorValue, conditionValue)) {
            allConditionsTrue = false;
            break;
        }
    }

    return hasConditions && allConditionsTrue;
}

#endif // SENSORS_H
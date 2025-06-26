
#ifndef RELAY_H
#define RELAY_H

#include "Arduino.h"

/**
 * @class Relay
 * @brief A class to control a relay connected to a GPIO pin
 */
class Relay {
public:
  /**
   * @brief Constructor for Relay class
   * @param pin The GPIO pin number (0-31)
   * @param relayState Initial state of the relay (true for ON, false for OFF)
   */
  Relay(const uint8_t pin, bool relayState = 0)
      : pin(pin), relayState(relayState) {
    // Validate pin (simplified)
    if (pin < 0 || pin > 31) {
      Serial.println("Error: Invalid pin number");
    }
    pinMode(pin, OUTPUT);
    digitalWrite(pin, relayState); // Set the relays state
  }

  /**
   * @brief Turn the relay on
   */
  void turnOn() {
    relayState = true;
    digitalWrite(pin, HIGH);
  }

  /**
   * @brief Turn the relay off
   */
  void turnOff() {
    relayState = false;
    digitalWrite(pin, LOW);
  }

  /**
   * @brief Check if the relay is currently on
   * @return true if relay is ON, false otherwise
   */
  bool isOn() const { return relayState; }

  /**
   * @brief Get the GPIO pin number
   * @return The pin number associated with this relay
   */
  const uint8_t getPin() const { return pin; }

private:
  /**
   * @var pin
   * @brief The GPIO pin number connected to the relay
   */
  const uint8_t pin;

  /**
   * @var relayState
   * @brief Current state of the relay (true for ON, false for OFF)
   */
  bool relayState;
};
#endif // RELAY_H
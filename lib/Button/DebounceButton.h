#ifndef DEBOUNCEBUTTON_H
#define DEBOUNCEBUTTON_H

#include <Arduino.h>

#define NO_INPUT 0
#define UP 1
#define DOWN 2
#define SELECT 3

class DebounceButton {
public:
  // Constructor
  DebounceButton(int buttonPin, unsigned long debounceDelay = 50);

  // Method to check if the button is pressed
  bool isPressed();
  uint8_t getInput();

private:
  const int buttonPin;                // Pin number for the button
  const unsigned long debounceDelay;  // Debounce time in milliseconds
  int buttonState = LOW;              // Current reading from the button
  int lastButtonState = LOW;          // Previous reading from the button
  unsigned long lastDebounceTime = 0; // Last time the button state was toggled
};

// for easy
#define BUTTON_UP_PIN 25
#define BUTTON_DOWN_PIN 33
#define BUTTON_SELECT_PIN 32

#endif // DEBOUNCEBUTTON_H

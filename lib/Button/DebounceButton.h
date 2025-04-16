#ifndef DEBOUNCEBUTTON_H
#define DEBOUNCEBUTTON_H

#include <Arduino.h>

class DebounceButton {
public:
  // Constructor
  DebounceButton(int buttonPin, unsigned long debounceDelay = 50);

  // Method to check if the button is pressed
  bool isPressed();

private:
  const int buttonPin;                // Pin number for the button
  const unsigned long debounceDelay;  // Debounce time in milliseconds
  int buttonState = LOW;              // Current reading from the button
  int lastButtonState = LOW;          // Previous reading from the button
  unsigned long lastDebounceTime = 0; // Last time the button state was toggled
};

#endif // DEBOUNCEBUTTON_H

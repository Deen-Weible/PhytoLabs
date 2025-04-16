#include "DebounceButton.h"

DebounceButton::DebounceButton(int buttonPin, unsigned long debounceDelay)
    : buttonPin(buttonPin), debounceDelay(debounceDelay) {
  pinMode(buttonPin, INPUT);
}

bool DebounceButton::isPressed() {
  int reading = digitalRead(buttonPin);

  // Debounce logic
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // Check if the button has been pressed for more than the debounce delay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        return true;
      }
    }
  }

  lastButtonState = reading;
  return false;
}
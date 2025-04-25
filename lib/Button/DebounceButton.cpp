#include "DebounceButton.h"

DebounceButton::DebounceButton(int buttonPin, unsigned long debounceDelay)
    : buttonPin(buttonPin), debounceDelay(debounceDelay), lastButtonState(HIGH),
      buttonState(HIGH), lastDebounceTime(0) {
  pinMode(buttonPin, INPUT_PULLUP);

  // ensure it doesn't register a press on startup
  int initialState = digitalRead(buttonPin);
  lastButtonState = initialState;
  buttonState = initialState;
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
#ifndef UIKIT_H
#define UIKIT_H

#include <Arduino.h>
#include <Helpers.h>
#include <U8g2lib.h>

// Class to manage navigation state between screens
// (commented out as placeholder for future implementation)
/*
class NavInfo {
public:
  // constructorf
  NavInfo(uint8_t s) : current_screen_id(s) {};

  void SetCurrentScreenId(uint8_t s) { current_screen_id = s; }
  uint8_t GetCurrentScreenId() const { return current_screen_id; }
  void SetCurrentScreen(Screen *s, uint8_t id) {
    current_screen = s;
    current_screen_id = id;
  }
  Screen *GetCurrentScreen() const { return current_screen; }

private:
  uint8_t current_screen_id;
  Screen *current_screen;
};
*/

// Base class for all UI screens
class Screen {
public:
  // Constructor with screen identifier
  Screen(uint8_t s) : screen_id(s) {}

  // Pure virtual function for drawing the screen content
  virtual void Draw() = 0;

  // Virtual function to handle user input events
  virtual void HandleInput(uint8_t input);

  // Getter for screen ID
  const uint8_t getId() const { return screen_id; }

private:
  const uint8_t screen_id; // Unique identifier for the screen
};

// Class representing a menu item with associated screen
class MenuItem {
public:
  // Constructor with default empty strings if nullptr is passed
  MenuItem(const char *Title, const char *Description,
           const unsigned char *Icon, const uint8_t id, Screen *screen)
      : title(Title ? Title : ""), description(Description ? Description : ""),
        icon(Icon), id(id), screen(screen) {}

  // Getter methods for menu item properties
  const char *GetTitle() const { return title; }
  const char *GetDescription() const { return description; }
  const unsigned char *GetIcon() const { return icon; }
  const uint8_t GetId() const { return id; }
  Screen *GetScreen() const { return screen; }

private:
  const char *title;        // Menu item title
  const char *description;  // Menu item description
  const unsigned char *icon; // Pointer to bitmap array for the icon
  uint8_t id;               // Unique identifier for the menu item
  Screen *screen;           // Associated screen object
};

// Class to manage current screen state and navigation
class NavInfo {
public:
  // constructorf
  NavInfo(uint8_t s) : current_screen_id(s) {}

  void SetCurrentScreenId(uint8_t s) { current_screen_id = s; }
  uint8_t GetCurrentScreenId() const { return current_screen_id; }
  void SetCurrentScreen(Screen *s, uint8_t id) {
    current_screen = s;
    current_screen_id = id;
  }
  Screen *GetCurrentScreen() const { return current_screen; }

private:
  uint8_t current_screen_id; // Current screen ID
  Screen *current_screen;    // Reference to the currently active screen
};

// Settings Menus
// MenuItem TimeMenu("Time", "Current time", Untitled_bits, 0);
// MenuItem SliderMenu("Slider", "Slider test", Untitled_bits, 1);
// MenuItem WiFiMenu("WiFi", "Ze' Wifi", Untitled_bits, 2);

// MenuItem SettingsMenus[3] = {TimeMenu, SliderMenu, WiFiMenu};

#endif // MY_H_FILE
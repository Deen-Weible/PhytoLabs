#ifndef UIKIT_H
#define UIKIT_H

#include <Arduino.h>
#include <Helpers.h>
#include <U8g2lib.h>

#define MAX_SCREENS 20

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
  virtual uint8_t HandleInput(uint8_t input);

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
  const char *title;         // Menu item title
  const char *description;   // Menu item description
  const unsigned char *icon; // Pointer to bitmap array for the icon
  uint8_t id;                // Unique identifier for the menu item
  Screen *screen;            // Associated screen object
};

// Class to manage current screen state and navigation
class NavInfo {
public:
  // Constructor: Initialize current screen and screen list
  NavInfo(uint8_t initialId)
      : current_screen_id(initialId), current_screen(nullptr), num_screens(0) {
    // Initialize all screen pointers to nullptr
    for (int i = 0; i < MAX_SCREENS; i++) {
      screens[i].screen = nullptr;
    }
  }

  // Register a screen with its ID
  void RegisterScreen(Screen *screen) {
    if (num_screens < MAX_SCREENS) {
      screens[num_screens].id = screen->getId();
      screens[num_screens].screen = screen;
      num_screens++;
    }
    // If full, do nothing (or add error handling if needed)
  }

  // Get a screen by its ID
  Screen *GetScreenById(uint8_t id) const {
    for (int i = 0; i < num_screens; i++) {
      if (screens[i].id == id) {
        return screens[i].screen;
      }
    }
    return nullptr; // Return nullptr if not found
  }

  // Set the current screen using its ID
  void SetScreenById(uint8_t id) {
      Serial.println("Setting screen");
      current_screen = GetScreenById(id);
      current_screen_id = id;
  }

  void test() {
    Serial.println("Test called");
  }

  // Other useful methods
  void SetCurrentScreen(Screen *screen) {
    current_screen = screen;
    current_screen_id = screen->getId();
  }
  Screen *GetCurrentScreen() const { return current_screen; }
  uint8_t GetCurrentScreenId() const { return current_screen_id; }

private:
  uint8_t current_screen_id; // Current screen ID (1 byte)
  Screen *current_screen;    // Current screen pointer (2 bytes)
  int num_screens;           // Number of registered screens (2 bytes)

  // Struct to store screen ID and pointer
  struct ScreenEntry {
    uint8_t id;           // 1 byte
    Screen *screen;       // 2 bytes
  } screens[MAX_SCREENS]; // Array of MAX_SCREENS entries
};
// class NavInfo {
// public:
//   // constructorf
//   NavInfo(uint8_t s) : current_screen_id(s) {}

//   void SetCurrentScreenId(uint8_t s) { current_screen_id = s; }
//   uint8_t GetCurrentScreenId() const { return current_screen_id; }
//   void SetCurrentScreen(Screen *s, uint8_t id) {
//     current_screen = s;
//     current_screen_id = id;
//   }
//   Screen *GetCurrentScreen() const { return current_screen; }

// private:
//   uint8_t current_screen_id; // Current screen ID
//   Screen *current_screen;    // Reference to the currently active screen
// };

// Settings Menus
// MenuItem TimeMenu("Time", "Current time", kPlaceholderIcon, 0);
// MenuItem SliderMenu("Slider", "Slider test", kPlaceholderIcon, 1);
// MenuItem WiFiMenu("WiFi", "Ze' Wifi", kPlaceholderIcon, 2);

// MenuItem SettingsMenus[3] = {TimeMenu, SliderMenu, WiFiMenu};

#endif // MY_H_FILE
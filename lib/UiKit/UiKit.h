#ifndef UIKIT_H
#define UIKIT_H

#include <Arduino.h>
#include <Helpers.h>
#include <U8g2lib.h>

#define MAX_SCREENS 20

/**
 * @brief Base class for all UI screens
 */
class Screen {
public:
  // Constructor with screen identifier
  Screen(uint8_t s) : screen_id(s) {}

  /**
   * @brief Pure virtual function for drawing the screen content
   * @note This method must be overridden by derived classes
   */
  virtual void Draw() = 0;

  /**
   * @brief Virtual function to handle user input events
   * @param input The input event code (e.g., button press)
   * @return uint8_t Action code indicating the result of handling input
   */
  virtual uint8_t HandleInput(uint8_t input);

  /**
   * @brief Getter for screen ID
   * @return const uint8_t The unique identifier for the screen
   */
  const uint8_t getId() const { return screen_id; }

private:
  const uint8_t screen_id; // Unique identifier for the screen
};

/**
 * @brief Class representing a menu item with associated screen
 */
class MenuItem {
public:
  /**
   * @brief Constructor with default empty strings if nullptr is passed
   * @param Title The title of the menu item
   * @param Description The description of the menu item
   * @param Icon Pointer to bitmap array for the icon
   * @param id Unique identifier for the menu item
   */
  MenuItem(const char *Title, const char *Description,
           const unsigned char *Icon, const uint8_t id)
      : title(Title ? Title : ""), description(Description ? Description : ""),
        icon(Icon), id(id) {}

  /**
   * @brief Getter methods for menu item properties
   * @return const char* The title of the menu item
   */
  const char *GetTitle() const { return title; }

  /**
   * @brief Getter methods for menu item properties
   * @return const char* The description of the menu item
   */
  const char *GetDescription() const { return description; }

  /**
   * @brief Getter methods for menu item properties
   * @return const unsigned char* Pointer to bitmap array for the icon
   */
  const unsigned char *GetIcon() const { return icon; }

  /**
   * @brief Getter methods for menu item properties
   * @return const uint8_t Unique identifier for the menu item
   */
  const uint8_t GetId() const { return id; }

private:
  const char *title;         // Menu item title
  const char *description;   // Menu item description
  const unsigned char *icon; // Pointer to bitmap array for the icon
  uint8_t id;                // Unique identifier for the menu item
};

/**
 * @brief Class to manage current screen state and navigation
 */
class NavInfo {
public:
  /**
   * @brief Constructor: Initialize current screen and screen list
   * @param initialId The ID of the initial screen
   */
  NavInfo(uint8_t initialId)
      : current_screen_id(initialId), current_screen(nullptr), num_screens(0) {
    // Initialize all screen pointers to nullptr
    for (int i = 0; i < MAX_SCREENS; i++) {
      screens[i].screen = nullptr;
    }
  }

  /**
   * @brief Register a screen with its ID
   * @param screen Pointer to the Screen object to register
   */
  void RegisterScreen(Screen *screen) {
    if (num_screens < MAX_SCREENS) {
      screens[num_screens].id = screen->getId();
      screens[num_screens].screen = screen;
      num_screens++;
    }
    // If full, do nothing (or add error handling if needed)
  }

  /**
   * @brief Get a screen by its ID
   * @param id The ID of the screen to retrieve
   * @return Screen* Pointer to the registered screen or nullptr if not found
   */
  Screen *GetScreenById(uint8_t id) const {
    for (int i = 0; i < num_screens; i++) {
      if (screens[i].id == id) {
        return screens[i].screen;
      }
    }
    return nullptr; // Return nullptr if not found
  }

  /**
   * @brief Set the current screen using its ID
   * @param id The ID of the screen to set as current
   */
  void SetScreenById(uint8_t id) {
      Serial.println("Setting screen");
      current_screen = GetScreenById(id);
      current_screen_id = id;
  }

  /**
   * @brief Test function for debugging purposes
   */
  void test() {
    Serial.println("Test called");
  }

  /**
   * @brief Set the current screen using a pointer
   * @param screen Pointer to the Screen object
   */
  void SetCurrentScreen(Screen *screen) {
    current_screen = screen;
    current_screen_id = screen->getId();
  }
  /**
   * @brief Get the current screen
   * @return Screen* Pointer to the current screen
   */
  Screen *GetCurrentScreen() const { return current_screen; }
  /**
   * @brief Get the current screen ID
   * @return uint8_t The ID of the current screen
   */
  uint8_t GetCurrentScreenId() const { return current_screen_id; }

private:
  uint8_t current_screen_id; // Current screen ID (1 byte)
  Screen *current_screen;    // Current screen pointer (2 bytes)
  int num_screens;           // Number of registered screens (2 bytes)

  /**
   * @brief Struct to store screen ID and pointer
   */
  struct ScreenEntry {
    uint8_t id;           // 1 byte
    Screen *screen;       // 2 bytes
  } screens[MAX_SCREENS]; // Array of MAX_SCREENS entries
};

// class NavInfo {
// public:
//   // constructorf
//   NavInfo(uint8_t s) : current_screen_id(s) {}
//
//   void SetCurrentScreenId(uint8_t s) { current_screen_id = s; }
//   uint8_t GetCurrentScreenId() const { return current_screen_id; }
//   void SetCurrentScreen(Screen *s, uint8_t id) {
//     current_screen = s;
//     current_screen_id = id;
//   }
//   Screen *GetCurrentScreen() const { return current_screen; }
//
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

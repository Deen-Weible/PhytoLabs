#ifndef UIKIT_H
#define UIKIT_H

#include <Arduino.h>
#include <Helpers.h>
#include <U8g2lib.h>

// class NavInfo {
// public:
//   // constructorf
//   NavInfo(uint8_t s) : current_screen_id(s) {};

//   uint8_t GetCurrentScreenId() const { return current_screen_id; }
//   void setCurrentScreenId(uint8_t s) { current_screen_id = s; }
//   Screen *GetCurrentScreen() const { return current_screen; }
//   void setCurrentScreen(Screen *s) { current_screen = s; }

// private:
//   uint8_t current_screen_id;
//   Screen *current_screen;
// };
class Screen {
  public:
    Screen(uint8_t s) : screen_id(s) {};

    virtual void Draw() = 0;
    virtual uint8_t HandleInput(uint8_t input);

    const uint8_t getId() const { return screen_id; }

  private:
    const uint8_t screen_id;
  };


class MenuItem {
public:
  // Constructor with default empty strings if nullptr is passed
  MenuItem(const char *Title, const char *Description,
           const unsigned char *Icon, const uint8_t id, Screen *screen)
      : title(Title ? Title : ""), description(Description ? Description : ""),
        icon(Icon), id(id), screen(screen) {}

  // Getter methods
  const char *GetTitle() const { return title; }
  const char *GetDescription() const { return description; }
  const unsigned char *GetIcon() const { return icon; }
  const uint8_t GetId() const { return id; }
  const Screen *GetScreen() const { return screen; }

private:
  const char *title;
  const char *description;
  const unsigned char *icon; // Pointer to bitmap array for the icon
  uint8_t id;
  const Screen *screen;
};

class NavInfo {
public:
  // constructorf
  NavInfo(uint8_t s) : current_screen_id(s) {};

  void setCurrentScreenId(uint8_t s) { current_screen_id = s; }
  uint8_t GetCurrentScreenId() const { return current_screen_id; }

  void SetCurrentScreen(Screen *s, uint8_t id) {}
  // void setCurrentScreen(Screen *s) { current_screen = s; }

  // Screen *GetCurrentScreen() const { return current_screen; }

private:
  uint8_t current_screen_id;
  Screen *current_screen;
};

// class Screen {
// public:
//   virtual ~Screen() {}
//   // Pure virtual function, must be implemented by derived classes
//   virtual void Draw() = 0;
// };

// Settings Menus

// MenuItem TimeMenu("Time", "Current time", Untitled_bits, 0);
// MenuItem SliderMenu("Slider", "Slider test", Untitled_bits, 1);
// MenuItem WiFiMenu("WiFi", "Ze' Wifi", Untitled_bits, 2);

// MenuItem SettingsMenus[3] = {TimeMenu, SliderMenu, WiFiMenu};

#endif // MY_H_FILE

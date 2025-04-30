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

class NavInfo {
public:
  // constructorf
  NavInfo(uint8_t s) : current_screen_id(s) {};

  uint8_t GetCurrentScreenId() const { return current_screen_id; }
  void setCurrentScreenId(uint8_t s) { current_screen_id = s; }
  Screen *GetCurrentScreen() const { return current_screen; }
  void setCurrentScreen(Screen *s) { current_screen = s; }

private:
  uint8_t current_screen_id;
  Screen *current_screen;
};

class Screen {
public:
  virtual void Draw() = 0;
  virtual uint8_t HandleInput(uint8_t input, NavInfo& navInfo) = NO_INPUT;
};

class TimeScreen : public Screen {
public:
  void Draw() override {
    // Debug
    u8g2.setFontMode(1);
    u8g2.setFont(u8g_font_baby);
    u8g2.setDrawColor(2);
    u8g2.drawStr(50, 50, "Time Screen");
  }
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
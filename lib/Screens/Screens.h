#ifndef SCREENS_H
#define SCREENS_H

// #include <U8g2lib.h>  // Assuming this is included elsewhere if needed
#include <Helpers.h>
#include <UiKit.h>

// Class representing a single menu item with title, description, icon, and ID

// // Class representing a list menu with multiple items
// class SettingsList {
// public:
//   // Constructor with pointer to menu items and number of items
//   SettingsList(MenuItem *items, size_t num_items)
//       : items(items), num_items(num_items), current_item(0) {}

//   // Update the menu with a new set of items and their count
//   void UpdateItems(MenuItem *new_items, size_t new_num_items) {
//     items = new_items;
//     num_items = new_num_items;
//   }

//   // Navigate up in the menu (wraps around)
//   void Up() { current_item = Wrap(current_item + 1, 0, num_items - 1); }

//   // Navigate down in the menu (wraps around)
//   void Down() { current_item = Wrap(current_item - 1, 0, num_items - 1); }

//   // Select the current item and return its ID
//   uint8_t Select() { return items[current_item].GetId(); }

//   // Draw the menu items on the display
//   void Draw() {
//     if (num_items < 2) {
//       // Handle case where there are fewer than 2 items
//       return;
//     }

//     const char *title1 = items[current_item].GetTitle();
//     const char *title2 = items[current_item + 1].GetTitle();
//     const unsigned char *icon1 = items[0].GetIcon();
//     const unsigned char *icon2 = items[current_item + 1].GetIcon();

//     if (title1 == nullptr || title2 == nullptr) {
//       Serial.println("Error: Null title");
//       return;
//     }

//     u8g2.setFontMode(1);
//     u8g2.setFont(u8g_font_7x13B);
//     u8g2.drawXBMP(5, 16, 14, 14, icon1);
//     u8g2.drawStr(25, 28, title1);

//     u8g2.setFont(u8g_font_5x8);
//     u8g2.drawXBMP(5, 33, 14, 14, icon2);
//     u8g2.drawStr(25, 44, title2);

//     // TODO: Indicate the selected item (e.g., with a cursor or highlight)
//   }

// private:
//   MenuItem *items;  // Pointer to the array of menu items
//   size_t num_items; // Number of items in the menu
//   int current_item; // Index of the currently selected item
// };

class SettingsList : public Screen {
public:
  // Added screen_id and num_items as parameters
  SettingsList(uint8_t screen_id, uint8_t num_items, MenuItem *items, NavInfo *nav_info,
           uint8_t new_current_item = 0)
      : Screen(screen_id),                  // Initialize the base class
        items(items), num_items(num_items), // Use the provided number of items
        nav_info(nav_info),                  // Initialize the NavInfo
        current_item(new_current_item) {}

  uint8_t HandleInput(uint8_t input) override {
    switch (input) {
    case UP:
      current_item = Wrap(current_item + 1, 0, num_items - 1);
      break;
    case DOWN:
      current_item = Wrap(current_item - 1, 0, num_items - 1);
      break;
    case SELECT:
      Serial.println("Selected: " + String(items[current_item].GetId()));
      nav_info->SetCurrentScreen(items[current_item].GetScreen(), items[current_item].GetId());
      return items[current_item].GetId();
      break;
    }
    return NULL;
  }

  void Draw() override {
    if (num_items < 2) {
      return;
    }

    u8g2.setFontMode(1);
    u8g2.setFont(u8g_font_7x13B);
    u8g2.drawXBMP(5, 16, 14, 14, items[0].GetIcon());
    u8g2.drawStr(25, 28, items[current_item].GetTitle());

    if (current_item + 1 < num_items) {
      u8g2.setFont(u8g_font_5x8);
      u8g2.drawXBMP(5, 33, 14, 14, items[current_item + 1].GetIcon());
      u8g2.drawStr(25, 44, items[current_item + 1].GetTitle());
    }
  }

private:
  MenuItem *items;      // Pointer to the array of menu items
  NavInfo *nav_info;   // Navigation information object
  size_t num_items;     // Number of items in the menu
  uint8_t current_item; // Index of the currently selected item
};

// Class representing a time screen, inheriting from Screen
class TimeScreen : public Screen {
public:
  // Override the Draw method to display the time screen
  void Draw() override {
    u8g2.setFontMode(1);
    u8g2.setFont(u8g_font_baby);
    u8g2.setDrawColor(2);
    u8g2.drawStr(50, 50, "Time Screen");
  }
};

// Class representing the base UI with title, description, and time
class BaseUi {
public:
  // Constructor for BaseUi
  BaseUi(char *t, char *d, InternalTime *c) : title(t), desc(d), time(c) {}

  // Update the title and description
  void update(char *new_title, char *new_desc) {
    title = new_title;
    desc = new_desc;
  }

  // Draw the base UI elements on the display
  void Draw() {
    u8g2.setFontMode(1);
    u8g2.drawBox(0, 0, SCREEN_WIDTH, 10);
    u8g2.setDrawColor(2);
    u8g2.setFont(u8g_font_baby);
    u8g2.drawStr(0, 7, title);

    // Draw the time if available
    if (time != NULL) {
      char time_str[20];
      sprintf(time_str, "%02d:%02d", time->get_hour(), time->get_minute());
      u8g2.drawStr(103, 7, time_str);
    }

    u8g2.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10);
    u8g2.drawStr(0, SCREEN_HEIGHT - 2, desc);
  }

private:
  char *title;        // Pointer to the title string
  char *desc;         // Pointer to the description string
  InternalTime *time; // Pointer to the internal time object
};

// Commented-out TestClass for reference
// class TestClass {
// public:
//   TestClass(BaseUi *ui) : base_ui(ui) {}
//   void setTitle() { base_ui->update("Test Title", "Test description"); }

// private:
//   BaseUi *base_ui;
// };

#endif // SCREENS_H
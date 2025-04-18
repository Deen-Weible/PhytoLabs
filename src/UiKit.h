#include <Arduino.h>
#include <Helpers.h>
#include <U8g2lib.h>

class BaseUi {
public:
  // initial constructor
  BaseUi(char *t, char *d, InternalTime *c) : title(t), desc(d), time(c) {}

  void update(char *new_title, char *new_desc) {
    title = new_title;
    desc = new_desc;
  }

  void draw() {
    u8g2.setFontMode(1);
    u8g2.drawBox(0, 0, SCREEN_WIDTH, 10);
    u8g2.setDrawColor(2);
    u8g2.setFont(u8g_font_baby);
    u8g2.drawStr(0, 7, title);

    // Ensure time pointer is valid (we hate DRAM vulnerabilities)
    if (time != NULL) {
      // Draw clock
      char time_str[20];
      sprintf(time_str, "%02d:%02d", time->get_hour(), time->get_minute());
      u8g2.drawStr(103, 7, time_str);
    }

    u8g2.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10);
    u8g2.drawStr(0, SCREEN_HEIGHT - 2, desc);
  };

private:
  char *title;
  char *desc;
  InternalTime *time;
};

class MenuItem {
public:
  MenuItem(const char *Title, const char *Description,
           const unsigned char *Icon)
      : title(Title ? Title : ""), description(Description ? Description : ""),
        icon(Icon) {}
  void draw(int x, int y) const; // Assuming read-only
  bool isPressed() const;        // Assuming read-only
  const char *GetTitle() const { return title; }
  const char *GetDescription() const { return description; }
  const unsigned char *GetIcon() const {
    return icon;
  } // Return pointer to array

private:
  const char *title;
  const char *description;
  const unsigned char *icon; // Changed to pointer for bitmap array
};

class ListMenu {
public:
  // Constructor with array of menu items
  ListMenu(MenuItem items[], int num_items)
      : items(items), num_items(num_items) {};

  // Update with a new array of menu items
  void UpdateItems(MenuItem new_items[]) {
    items = new_items;
    num_items = sizeof(new_items) / sizeof(new_items[0]);
  }

  void Up() { current_item = Wrap((current_item + 1), 0, num_items - 1); }
  void Down() { current_item = Wrap((current_item - 1), 0, num_items - 1); }


private:
  MenuItem *items; // Use a pointer to manage dynamic memory
  int current_item = 0;
  int num_items;
};
// class ListMenu {
// public:
//   // constructor with two menu items
//   ListMenu(MenuItem &item1, MenuItem &item2)
//       : item1(item1), item2(item2) {}

//   void UpdateItems(MenuItem &new_item1, MenuItem &new_item2) {
//     item1 = new_item1;
//     item2 = new_item2;
//   }

//   // draw the list menu
//   void Draw() {
//     const char *title = item1.GetTitle();
//     const char *title2 = item2.GetTitle();
//     const unsigned char *icon1 = item1.GetIcon(); // Pointer to bitmap array
//     const unsigned char *icon2 = item2.GetIcon(); // Pointer to bitmap array

//     if (title == nullptr) {
//       Serial.println("Error: Null title");
//       return;
//     }
//     u8g2.setFontMode(1);

//     u8g2.setFont(u8g_font_7x13B);
//     u8g2.drawXBMP(5, 16, 14, 14, icon1); // Use bitmap array
//     u8g2.drawStr(25, 28, title);

//     u8g2.setFont(u8g_font_5x8);
//     u8g2.drawBitmap(4, 33, 16 / 8, 16, icon2); // Use bitmap array
//     u8g2.drawStr(25, 44, title2);
//   }

// private:
//   MenuItem &item1;
//   MenuItem &item2;
// };
#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <icons.h>
#include <Preferences.h>

// Initialize preferences
Preferences preferences;

// Display settings
U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2(U8G2_R0, 18, 23, 15, 17, 16);
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define CLOCK_SPEED 400000

// Button pins
#define BUTTON_DOWN_PIN 25
#define BUTTON_SELECT_PIN 32
#define BUTTON_UP_PIN 33

int button_down_clicked = 0;
int button_select_clicked = 0;
int button_up_clicked = 0;

// Menu configuration
int item_selected = 0;
const int NUM_ITEMS = 8;
const int MAX_ITEM_LENGTH = 20;

// Keep track of menu depth
int current_screen = 0;

// Menu items and descriptions
char menu_items[NUM_ITEMS][MAX_ITEM_LENGTH] = {
    {"3D Cube"}, {"Battery"}, {"Dashboard"}, {"Fireworks"}, {"GPS Speed"}, {"Big Knob"}, {"Park Sensor"}, {"Turbo Gauge"}};

char item_descriptions[NUM_ITEMS][MAX_ITEM_LENGTH] = {
    {"desc 1"}, {"desc 2"}, {"desc 3"}, {"desc 4"}, {"desc 5"}, {"desc 6"}, {"desc 7"}, {"desc 8"}};

// Draw basic UI elements
void drawUI(const char *title, const char *description)
{
  u8g2.setFontMode(1);
  u8g2.drawBox(0, 0, SCREEN_WIDTH, 10);
  u8g2.setDrawColor(2);
  u8g2.setFont(u8g_font_baby);
  u8g2.drawStr(0, 7, title);

  u8g2.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10);
  u8g2.drawStr(0, SCREEN_HEIGHT - 2, description);
}

// Draw individual menu items
void drawItem(int position, const char *label)
{
  int textY = (position == 1) ? 28 : 44;
  int iconY = (position == 1) ? 15 : 33;

  u8g2.drawStr(25, textY, label);
  u8g2.drawBitmap(4, iconY, 16 / 8, 16, Square);
}

// Update display content
void updateDisplay()
{
  u8g2.firstPage();
  do
  {
    drawUI("Settings", item_descriptions[item_selected]);

    if (current_screen == 0)
    { // Menu Screen
      // Handle button inputs
      if ((digitalRead(BUTTON_UP_PIN) == LOW) && !button_up_clicked)
      {
        --item_selected;
        button_up_clicked = 1;
        preferences.putUInt("item_selected", item_selected);

        if (item_selected < 0)
          item_selected = NUM_ITEMS - 1;
      }
      else if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && !button_down_clicked)
      {
        ++item_selected;
        button_down_clicked = 1;
        preferences.putUInt("item_selected", item_selected);

        if (item_selected >= NUM_ITEMS)
          item_selected = 0;
      }

      // Update button states
      if ((digitalRead(BUTTON_UP_PIN) == HIGH))
        button_up_clicked = 0;
      if ((digitalRead(BUTTON_DOWN_PIN) == HIGH))
        button_down_clicked = 0;

      // Draw menu items
      u8g2.setFont(u8g_font_7x13B);
      drawItem(1, menu_items[item_selected]);

      if (item_selected < NUM_ITEMS - 1)
      {
        u8g2.setFont(u8g_font_5x8);
        drawItem(2, menu_items[item_selected + 1]);
      }
    }

  } while (u8g2.nextPage());
}

// Setup function
void setup()
{
  preferences.begin("settings", false);
  item_selected = preferences.getUInt("item_selected");
  u8g2.begin();

  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  SPI.setClockDivider(CLOCK_SPEED);
  u8g2.setBusClock(CLOCK_SPEED);
}

// Main loop
void loop()
{
  updateDisplay();
}

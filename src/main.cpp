#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <icons.h>
#include <Preferences.h>
#include <time.h>
// #include <sys/time.h>
// #include <ESP32Time.h>

// struct tm tmstruct = {0, 0, 0, 0, 0, 0, 0, 0, 0};
// tm *offsetTime = 0;
// struct tm rtc;
time_t now;

// Initialize preferences
Preferences preferences;

// Display settings
U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2(U8G2_R0, 18, 23, 15, 17, 16);
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define CLOCK_SPEED 400000

// Button pins
#define BUTTON_DOWN_PIN 33
#define BUTTON_SELECT_PIN 32
#define BUTTON_UP_PIN 25

int button_down_clicked = 0;
int button_select_clicked = 0;
int button_up_clicked = 0;

// Menu configuration
int item_selected = 0;
const int NUM_ITEMS = 8;
const int MAX_ITEM_LENGTH = 20;

// Keep track of menu depth
String current_screen = "settings";

// Time menu vars
int updated_hour = 0;
int updated_minute = 0;
int current_time_unit = 0;
long offset_time = 0;

// Current time values
int second = 0;
int minute = 0;
int hour = 0;


// Menu items and descriptions
char menu_items[NUM_ITEMS][MAX_ITEM_LENGTH] = {
    {"Time"}, {"Battery"}, {"Dashboard"}, {"Fireworks"}, {"GPS Speed"}, {"Big Knob"}, {"Park Sensor"}, {"Turbo Gauge"}};

char item_descriptions[NUM_ITEMS][MAX_ITEM_LENGTH] = {
    {"Current Time"}, {"desc 2"}, {"desc 3"}, {"desc 4"}, {"desc 5"}, {"desc 6"}, {"desc 7"}, {"desc 8"}};

// Wrap a time unit (0-59)
int wrapTime(int value)
{
  if (value < 60)
  {
    return value;
  }
  else if (value < 0)
  {
    return 59;
  }
  else
  {
    return 00;
  }
}

// void setTime(int sc, int mn, int hr, int dy, int mt, int yr) {
//   // seconds, minute, hour, day, month, year $ microseconds(optional)
//   // ie setTime(20, 34, 8, 1, 4, 2021) = 8:34:20 1/4/2021
//   struct tm t = {0, 0, 0, 0, 0, 0, 0, 0, 0};      // Initialize to all 0's
//   t.tm_hour = hr;
//   t.tm_min = mn;
//   time_t timeSinceEpoch = mktime(&t);
//   setTime(timeSinceEpoch, 0);
// }

// Helper functions for wrapping

int Wrap(int kX, int const kLowerBound, int const kUpperBound)
{
    int range_size = kUpperBound - kLowerBound + 1;

    if (kX < kLowerBound)
        kX += range_size * ((kLowerBound - kX) / range_size + 1);

    return kLowerBound + (kX - kLowerBound) % range_size;
}

int wrapMinute(int minute)
{
  if (minute >= 60)
    return 0;
  else if (minute < 0)
    return 59;
  return minute;
}

int wrapHour(int hour)
{
  if (hour >= 24)
    return 0;
  else if (hour < 0)
    return 23;
  return hour;
}

long secondsToHours(double seconds)
{
  return floor(seconds / 3600);
}

long secondsToMinutes(double seconds)
{
  return floor(seconds / 60);
}

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
    offset_time = time(&now) + updated_hour * 3600 + updated_minute * 60;
    second = Wrap(offset_time, 0, 59);
    minute = Wrap(secondsToMinutes(offset_time), 0, 59);
    hour = Wrap(secondsToHours(offset_time), 0, 23);
    // offset_time = 3720;
    // Update button states
    if ((digitalRead(BUTTON_UP_PIN) == HIGH))
    {
      button_up_clicked = 0;
    }
    if ((digitalRead(BUTTON_DOWN_PIN) == HIGH))
    {
      button_down_clicked = 0;
    }
    if ((digitalRead(BUTTON_SELECT_PIN) == HIGH))
    {
      button_select_clicked = 0;
    }

    if (current_screen == "settings")
    { // Menu Screen
      drawUI("Settings", item_descriptions[item_selected]);
      // Handle button inputs
      if ((digitalRead(BUTTON_UP_PIN) == LOW) && !button_up_clicked)
      {
        ++item_selected;
        button_up_clicked = 1;
        preferences.putUInt("item_selected", item_selected);

        if (item_selected < 0)
          item_selected = NUM_ITEMS - 1;
      }
      else if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && !button_down_clicked)
      {
        --item_selected;
        button_down_clicked = 1;
        preferences.putUInt("item_selected", item_selected);

        if (item_selected >= NUM_ITEMS)
          item_selected = 0;
      }
      else if ((digitalRead(BUTTON_SELECT_PIN) == LOW) && !button_select_clicked)
      {
        button_select_clicked = 1;
        current_screen = menu_items[item_selected];
      }

      // Draw menu items
      u8g2.setFont(u8g_font_7x13B);
      drawItem(1, menu_items[item_selected]);

      if (item_selected < NUM_ITEMS - 1)
      {
        u8g2.setFont(u8g_font_5x8);
        drawItem(2, menu_items[item_selected + 1]);
      }
    }
    else if (current_screen == "Time")
    {
      drawUI("Time", "Yeah, It's the time!");

      char buffer[10];
      char secondsBuffer[5];
      // if (offsetTime != 0)
      // {
      //   getLocalTime(&tmstruct, 0);
      //   // offsetTime += tmstruct;
      //   // tmstruct.tm_hour += offsetTime->tm_hour;
      // } // Changed tmstruct to tm_struct for clarity
      // tmstruct.tm_min = 20;

      if (current_time_unit < 2)
      {
        sprintf(buffer, "%02d:%02d", updated_hour, updated_minute);
      }
      else
      {
        sprintf(buffer, "%02d:%02d", hour, minute);
        // Serial.println(buffer);
      }
      sprintf(secondsBuffer, "%02d", second);

      u8g2.setFont(u8g_font_10x20r);

      if (current_time_unit == 0)
      {
        u8g2.drawBox(65, 16, 20, 19);
      }
      else if (current_time_unit == 1)
      {
        u8g2.drawBox(35, 16, 20, 19);
      }

      u8g2.drawStr(35, 32, buffer);
      u8g2.setFont(u8g_font_baby);
      u8g2.drawStr(85, 32, secondsBuffer);

      // Button handling
      if ((digitalRead(BUTTON_UP_PIN) == LOW) && !button_up_clicked)
      {
        button_up_clicked = 1;

        if (current_time_unit == 0) // Minutes mode
          updated_minute = wrapMinute(updated_minute + 1);
        else if (current_time_unit == 1) // Hours mode
          updated_hour = wrapHour(updated_hour + 1);
      }
      else if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && !button_down_clicked)
      {
        button_down_clicked = 1;

        if (current_time_unit == 0) // Minutes mode
          updated_minute = wrapMinute(updated_minute - 1);
        else if (current_time_unit == 1) // Hours mode
          updated_hour = wrapHour(updated_hour - 1);
      }
      else if ((digitalRead(BUTTON_SELECT_PIN) == LOW) && !button_select_clicked)
      {
        button_select_clicked = 1;
        current_time_unit++;
      }
    }

  } while (u8g2.nextPage());
}

// Setup function
void setup()
{
  Serial.begin(115200);
  SPI.setClockDivider(CLOCK_SPEED);
  u8g2.setBusClock(CLOCK_SPEED);

  // getLocalTime(&tmstruct, 0); // Get current time
  delay(2000);

  preferences.begin("settings", false);
  item_selected = preferences.getUInt("item_selected");

  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);

  u8g2.begin();
}

// Main loop
void loop()
{
  updateDisplay();
}

#ifndef HELPERS
#define HELPERS

#include <Arduino.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <U8g2lib.h>
#include <UiKit.h>

// Networking & Server Configuration
#define MAX_CLIENTS 4   // Maximum number of WiFi clients
#define WIFI_CHANNEL 6  // WiFi channel for access point
#define DNS_INTERVAL 30 // DNS server processing interval (ms) to prevent
const IPAddress localIP(1, 2, 3, 4);           // Local IP address for AP
const IPAddress gatewayIP(1, 2, 3, 4);         // Gateway IP (same as localIP)
const IPAddress subnetMask(255, 255, 255, 0);  // Subnet mask
const char *localUrl = "http://settings.dev";  // Local URL for easy access
const char *ssid = "PhytoLabs Initialization"; // AP SSID
const char *password = NULL; // AP password (NULL for open network)

// Web Server and DNS
DNSServer dnsServer;
AsyncWebServer server(80);

// Global Variables

time_t now;              // Current system time
Preferences preferences; // Persistent storage
U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2(U8G2_R0, 18, 23, 15, 17,
                                             16); // Display object
#define SCREEN_WIDTH 128                          // Display width in pixels
#define SCREEN_HEIGHT 64                          // Display height in pixels
#define CLOCK_SPEED 400000                        // SPI clock speed

// Menu Configuration
const int kMenuNumItems = 2; // Number of menu items
const int KMenuMaxTitleLength =
    22; // Maximum length of menu titles/descriptions

// Square Icon Bitmap (16x16)
const unsigned char kSquareIcon[] PROGMEM = {
    0x00, 0x00, 0x3f, 0xfc, 0x5f, 0xfa, 0x6f, 0xf6, 0x77, 0xee, 0x7b,
    0xde, 0x7c, 0x3e, 0x7c, 0x3e, 0x7c, 0x3e, 0x7c, 0x3e, 0x7b, 0xde,
    0x77, 0xee, 0x6f, 0xf6, 0x5f, 0xfa, 0x3f, 0xfc, 0x00, 0x00};

static unsigned char Untitled_bits[] PROGMEM = {
    0xfe, 0x1f, 0x03, 0x30, 0x09, 0x24, 0x01, 0x20, 0x01, 0x20,
    0xe1, 0x21, 0x11, 0x22, 0x11, 0x22, 0xe1, 0x21, 0x01, 0x20,
    0x01, 0x20, 0x01, 0x20, 0x03, 0x30, 0xfe, 0x1f};

// // Menu items and descriptions
// char menu_items[kMenuNumItems][KMenuMaxTitleLength] = {
//     {"Time"},      {"Slider Test"}, {"WiFi"},        {"Fireworks"},
//     {"GPS Speed"}, {"Big Knob"},    {"Park Sensor"}, {"Turbo Gauge"}};

// char menu_item_descriptions[kMenuNumItems][KMenuMaxTitleLength] = {
//     {"Current Time"}, {"Test ui slider"}, {"Manage WiFi / HotSpot"},
//     {"desc 4"},       {"desc 5"},         {"desc 6"},
//     {"desc 7"},       {"desc 8"}};

// Utility Functions
int Wrap(int kX, int const kLowerBound,
         int const kUpperBound);   // Wraps a value within a range
int SecondsToMinutes(int seconds); // Converts seconds to minutes (0-59)
int SecondsToHours(int seconds);   // Converts seconds to hours (0-23)
int WrapSeconds(int seconds);      // Wraps seconds within 0-59
int Clamp(int value, int lower_bound,
          int upper_bound); // Clamps a value within a range

// Base Menu Class for UI Screens
class Menu {
public:
  virtual void draw() = 0;        // Draw the menu on the display
  virtual void handleInput() = 0; // Handle user input
  virtual ~Menu() {}              // Virtual destructor for proper cleanup
};

// Range Class
class Range {
public:
  int min;
  int max;
};

// Helper to keep track of time, with offsets and shiz
class InternalTime {
public:
  void tick() { second = time(&now); }

  void set_current_time(int new_hour, int new_minute) {
    hour = new_hour;
    minute = new_minute;
    resetRTC();
  }

  void set_hour(int new_hour) {
    hour = new_hour;
    resetRTC();
  }
  void set_minute(int new_minute) {
    minute = new_minute;
    resetRTC();
  }

  // Reset the RTC to the offset time
  void resetRTC() {
    struct timeval tv = {.tv_sec = ((minute * 60) + (hour * 3600)),
                         .tv_usec = 0};
    settimeofday(&tv, NULL);
  }
  void set_epoch(long epoch) {
    struct timeval tv = {.tv_sec = epoch, .tv_usec = 0};
    settimeofday(&tv, NULL);
  }

  int get_hour() { return second / 3600 % 24; }
  int get_minute() { return (second % 3600) / 60; }
  int get_second() { return second % 60; }

private:
  long second = 0;
  int minute = 0;
  int hour = 0;
};

#endif
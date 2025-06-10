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
// Manages system time with hour, minute, and second tracking, including RTC
// synchronization
class InternalTime {
public:
  // Updates the internal second counter based on system time
  void Tick() { second_ = time(&now_); };

  // Sets the current time with hour and minute, resetting RTC
  void SetCurrentTime(int new_hour, int new_minute) {
    hour_ = new_hour;
    minute_ = new_minute;
    ResetRTC();
  };

  // Sets the hour and resets RTC
  void SetHour(int new_hour) {
    hour_ = new_hour;
    ResetRTC();
  };

  // Sets the minute and resets RTC
  void SetMinute(int new_minute) {
    minute_ = new_minute;
    ResetRTC();
  };

  // Resets the RTC based on the current hour and minute
  void ResetRTC() {
    struct timeval tv = {.tv_sec = (minute_ * 60) + (hour_ * 3600),
                         .tv_usec = 0};
    settimeofday(&tv, nullptr);
  };

  // Sets the system time using an epoch timestamp
  void SetEpoch(long epoch) {
    struct timeval tv = {.tv_sec = epoch, .tv_usec = 0};
    settimeofday(&tv, nullptr);
  };

  // Returns the current hour (0-23)
  int GetHour() const { return (second_ / 3600) % 24; };

  // Returns the current minute (0-59)
  int GetMinute() const { return (second_ % 3600) / 60; };

  // Returns the current second (0-59)
  int GetSecond() const { return second_ % 60; };

private:
  long second_ = 0; // Current time in seconds since epoch
  int minute_ = 0;  // Current minute (0-59)
  int hour_ = 0;    // Current hour (0-23)
  time_t now_;      // Current system time
};

#endif
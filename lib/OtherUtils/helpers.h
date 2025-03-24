#include <Arduino.h>
#include <U8g2lib.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

#ifndef HELPERS
#define HELPERS

// Networking & Sever
#define MAX_CLIENTS 4
#define WIFI_CHANNEL 6
#define DNS_INTERVAL 30 // to keep from overloading the DNS server
const IPAddress localIP(1, 2, 3, 4);
const IPAddress gatewayIP(1, 2, 3, 4); // same as localIP
const IPAddress subnetMask(255, 255, 255, 0);
const char *localUrl = "http://settings.dev"; // for ease of access
const char *ssid = "PhytoLabs Initialization";
const char *password = NULL; // no password

// Web server
DNSServer dnsServer;
AsyncWebServer server(80);

// Variables
time_t now;

Preferences preferences;

U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2(U8G2_R0, 18, 23, 15, 17, 16);
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define CLOCK_SPEED 400000

#define BUTTON_UP_PIN 25
#define BUTTON_DOWN_PIN 33
#define BUTTON_SELECT_PIN 32

#define UP_CONDITION (digitalRead(BUTTON_UP_PIN) == LOW) && !button_up_clicked
#define DOWN_CONDITION (digitalRead(BUTTON_DOWN_PIN) == LOW) && !button_down_clicked
#define SELECT_CONDITION (digitalRead(BUTTON_SELECT_PIN) == LOW) && !button_select_clicked

const int kMenuNumItems = 8;
const int KMenuMaxTitleLength = 22;

const unsigned char kSquareIcon[] PROGMEM = {
	0x00, 0x00, 0x3f, 0xfc, 0x5f, 0xfa, 0x6f, 0xf6, 0x77, 0xee, 0x7b, 0xde, 0x7c, 0x3e, 0x7c, 0x3e,
	0x7c, 0x3e, 0x7c, 0x3e, 0x7b, 0xde, 0x77, 0xee, 0x6f, 0xf6, 0x5f, 0xfa, 0x3f, 0xfc, 0x00, 0x00};


// Menu items and descriptions
char menu_items[kMenuNumItems][KMenuMaxTitleLength] = {
	{"Time"}, {"Slider Test"}, {"WiFi"}, {"Fireworks"}, {"GPS Speed"}, {"Big Knob"}, {"Park Sensor"}, {"Turbo Gauge"}};

char menu_item_descriptions[kMenuNumItems][KMenuMaxTitleLength] = {
	{"Current Time"}, {"Test ui slider"}, {"Manage WiFi / HotSpot"}, {"desc 4"}, {"desc 5"}, {"desc 6"}, {"desc 7"}, {"desc 8"}};

// Functions
// Function to wrap a number within a given range [kLowerBound, kUpperBound]
int Wrap(int kX, int const kLowerBound, int const kUpperBound);

// Converts seconds to minutes and wraps the result within [0, 59]
int SecondsToMinutes(int seconds);

// Converts seconds to hours and wraps the result within [0, 23]
int SecondsToHours(int seconds);

// Wraps seconds within [0, 59]
int WrapSeconds(int seconds);

// Clamps a value within a given range [kLowerBound, kUpperBound]
int Clamp(int value, int lower_bound, int upper_bound);
#endif

// Classes

class Range {
	public:
		int min;
		int max;
};
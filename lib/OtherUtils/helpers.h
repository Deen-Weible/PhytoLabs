#include <Arduino.h>
#include <U8g2lib.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <index.h>

#ifndef HELPERS
#define HELPERS

// Networking & Server Configuration
#define MAX_CLIENTS 4													 // Maximum number of WiFi clients
#define WIFI_CHANNEL 6												 // WiFi channel for access point
#define DNS_INTERVAL 30												 // DNS server processing interval (ms) to prevent overload
const IPAddress localIP(1, 2, 3, 4);					 // Local IP address for AP
const IPAddress gatewayIP(1, 2, 3, 4);				 // Gateway IP (same as localIP)
const IPAddress subnetMask(255, 255, 255, 0);	 // Subnet mask
const char *localUrl = "http://settings.dev";	 // Local URL for easy access
const char *ssid = "PhytoLabs Initialization"; // AP SSID
const char *password = NULL;									 // AP password (NULL for open network)

// Web Server and DNS
DNSServer dnsServer;
AsyncWebServer server(80);

// Global Variables
time_t now;																																 // Current system time
Preferences preferences;																									 // Persistent storage
U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2(U8G2_R0, 18, 23, 15, 17, 16); // Display object
#define SCREEN_WIDTH 128																									 // Display width in pixels
#define SCREEN_HEIGHT 64																									 // Display height in pixels
#define CLOCK_SPEED 400000																								 // SPI clock speed

// Button Pins and Conditions
#define BUTTON_UP_PIN 25
#define BUTTON_DOWN_PIN 33
#define BUTTON_SELECT_PIN 32
#define UP_CONDITION (digitalRead(BUTTON_UP_PIN) == LOW) && !button_up_clicked
#define DOWN_CONDITION (digitalRead(BUTTON_DOWN_PIN) == LOW) && !button_down_clicked
#define SELECT_CONDITION (digitalRead(BUTTON_SELECT_PIN) == LOW) && !button_select_clicked

// Menu Configuration
const int kMenuNumItems = 8;				// Number of menu items
const int KMenuMaxTitleLength = 22; // Maximum length of menu titles/descriptions

// Square Icon Bitmap (16x16)
const unsigned char kSquareIcon[] PROGMEM = {
		0x00, 0x00, 0x3f, 0xfc, 0x5f, 0xfa, 0x6f, 0xf6, 0x77, 0xee, 0x7b, 0xde, 0x7c, 0x3e, 0x7c, 0x3e,
		0x7c, 0x3e, 0x7c, 0x3e, 0x7b, 0xde, 0x77, 0xee, 0x6f, 0xf6, 0x5f, 0xfa, 0x3f, 0xfc, 0x00, 0x00};

// Menu items and descriptions
char menu_items[kMenuNumItems][KMenuMaxTitleLength] = {
		{"Time"}, {"Slider Test"}, {"WiFi"}, {"Fireworks"}, {"GPS Speed"}, {"Big Knob"}, {"Park Sensor"}, {"Turbo Gauge"}};

char menu_item_descriptions[kMenuNumItems][KMenuMaxTitleLength] = {
		{"Current Time"}, {"Test ui slider"}, {"Manage WiFi / HotSpot"}, {"desc 4"}, {"desc 5"}, {"desc 6"}, {"desc 7"}, {"desc 8"}};

// Utility Functions
int Wrap(int kX, int const kLowerBound, int const kUpperBound); // Wraps a value within a range
int SecondsToMinutes(int seconds);															// Converts seconds to minutes (0-59)
int SecondsToHours(int seconds);																// Converts seconds to hours (0-23)
int WrapSeconds(int seconds);																		// Wraps seconds within 0-59
int Clamp(int value, int lower_bound, int upper_bound);					// Clamps a value within a range

// Base Menu Class for UI Screens
class Menu
{
public:
	virtual void draw() = 0;				// Draw the menu on the display
	virtual void handleInput() = 0; // Handle user input
	virtual ~Menu() {}							// Virtual destructor for proper cleanup
};

// Range Class
class Range
{
public:
	int min;
	int max;
};

class InternalTime
{
public:
	long offset_time = 0;
	int updated_hour = 0;
	int updated_minute = 0;
	int updated_offset = 0;

	int current_second = 0;
	int current_minute = 0;
	int current_hour = 0;
};

class ServerConfigurator
{
public:
	ServerConfigurator(AsyncWebServer &server, const IPAddress &localIP)
			: _server(server), _localIP(localIP)
	{
	}

	void setupEndpoints()
	{
		_server.on("/wpad.dat", [](AsyncWebServerRequest *request)
							 { request->send(404); });
		_server.on("/connecttest.txt", [](AsyncWebServerRequest *request)
							 { request->redirect("http://logout.net"); });
		_server.on("/redirect", [](AsyncWebServerRequest *request)
							 { request->redirect(localUrl); });
		_server.on("/generate_204", [](AsyncWebServerRequest *request)
							 { request->redirect(localUrl); });
		_server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request)
							 { request->redirect(localUrl); });
		_server.on("/ncsi.txt", [](AsyncWebServerRequest *request)
							 { request->redirect(localUrl); });
		_server.on("/success.txt", [](AsyncWebServerRequest *request)
							 { request->send(200); });
		_server.on("/canonical.html", [](AsyncWebServerRequest *request)
							 { request->redirect(localUrl); });

		// 404 favicon
		_server.on("/favicon.ico", [](AsyncWebServerRequest *request)
							 { request->send(404); });

		// Serve the main page
		server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
							{
		AsyncWebServerResponse *response = request->beginResponse(200, "text/html", MAIN_page);
		response->addHeader("Cache-Control", "public,max-age=31536000");
		request->send(response); });

		server.on("/SendForms", [](AsyncWebServerRequest *request)
							{
		String response = request->getParam(0)->value();
		request->send(200, "text/plain", response);

		deserializeJson(doc, response);
	});
	}

private:
	AsyncWebServer &_server;
	IPAddress _localIP;
};

#endif
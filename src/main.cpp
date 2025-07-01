/**
 * @file main.cpp
 * @brief Main Arduino sketch for a device with OLED display and web interface
 */

/**
 * --- Standard Arduino and Library Includes ---
 */
#include <Arduino.h>     // Core Arduino library for basic functions
#include <ArduinoJson.h> // For JSON parsing and serialization
#include <Preferences.h> // For storing persistent data
#include <SPI.h>         // For SPI communication with display
#include <U8g2lib.h>     // For OLED display control
#include <Update.h>      // For OTA (Over-The-Air) updates
#include <time.h>        // For time-related functions

/**
 * --- Internal Project Headers ---
 */
#include <DebounceButton.h> // For debouncing button inputs
#include <Helpers.h>        // Helper functions for the project
#include <Icons.h>          // Icon definitions for UI
#include <Screens.h>        // Screen management classes
#include <UiKit.h>          // UI toolkit for display
#include <WiFiInfo.h>       // WiFi information class
#include <index.h>          // HTML content for the web server

/**
 * --- Global Variables ---
 */
String current_screen = "Settings"; // Tracks the currently displayed screen
InternalTime internal_time;         // Manages internal time with user offset
NavInfo nav_info(0); // Navigation info object initialized with ID 0

/**
 * --- Hack Screen Class (Temporary/Debugging) ---
 * @brief A temporary screen class for debugging purposes
 */
class hack_screen : public Screen {
public:
  /**
   * @brief Constructor with default screen ID 0
   */
  hack_screen() : Screen(0) {}

  /**
   * @brief Prints hack test as a stand-in
   */
  void Draw() override { Serial.println("Hack Screen has been summoned"); }

  /**
   * @brief Handles input for the hack screen
   * @param input The input type (UP, DOWN, SELECT, etc.)
   * @return The result of handling the input
   */
  uint8_t HandleInput(uint8_t input) override {
    Serial.println("Hack Screen input: " + String(input));
    return 0;
  }
};

hack_screen new_hack_screen;                      // Instance of hack_screen
TimeMenu time_menu(&internal_time, &nav_info, 2); // Time menu screen
SliderMenu slider_menu(&nav_info, 3);

/**
 * --- Menu Configuration ---
 */
const int kMenuNumItems = 8;        // Total number of menu items
const int KMenuMaxTitleLength = 22; // Max length for menu titles/descriptions

// Array of menu items (items beyond index 2 are placeholders)
MenuItem menuItems[kMenuNumItems] = {
    MenuItem("Time", "Current Time", kClockIcon, 2), // Time display
    MenuItem("Slider Test", "Test ui slider", kPlaceholderIcon,
             3), // UI slider test
    MenuItem("WiFi", "Manage WiFi / HotSpot", kPlaceholderIcon,
             2),                                            // WiFi settings
    MenuItem("Fireworks", "desc 4", kPlaceholderIcon, 3),   // Placeholder
    MenuItem("GPS Speed", "desc 5", kPlaceholderIcon, 4),   // Placeholder
    MenuItem("Big Knob", "desc 6", kPlaceholderIcon, 5),    // Placeholder
    MenuItem("Park Sensor", "desc 7", kPlaceholderIcon, 6), // Placeholder
    MenuItem("Turbo Gauge", "desc 8", kPlaceholderIcon, 7)  // Placeholder
};

/**
 * --- Button Objects for Debouncing ---
 */
DebounceButton upButton(BUTTON_UP_PIN);         // Button for "up" navigation
DebounceButton downButton(BUTTON_DOWN_PIN);     // Button for "down" navigation
DebounceButton selectButton(BUTTON_SELECT_PIN); // Button for selection

/**
 * --- Additional Global Variables ---
 */
// int current_setting_unit = 0; // Tracks which time unit is being adjusted
// (e.g., minutes, hours)
int selected_menu_item = 0; // Index of the currently selected menu item
JsonDocument doc;           // JSON document for processing form data

/**
 * @brief Detects button input and returns the type
 * @return The type of input (UP, DOWN, SELECT, or NO_INPUT)
 */
uint8_t getInput() {
  if (upButton.isPressed()) {
    return UP;
  } // Return UP if up button pressed
  if (downButton.isPressed()) {
    return DOWN;
  } // Return DOWN if down button pressed
  if (selectButton.isPressed()) {
    return SELECT;
  } // Return SELECT if select button pressed
  return NO_INPUT; // Return no input if no buttons pressed
}

/**
 * @brief Setup HTTP server for captive portal functionality
 * @param server The AsyncWebServer instance to configure
 * @param localIP The local IP address of the device
 */
void SetupServer(AsyncWebServer &server, const IPAddress &localIP) {
  // Redirect or handle specific URLs for captive portal compatibility
  server.on("/wpad.dat", [](AsyncWebServerRequest *request) {
    request->send(404);
  }); // Proxy autoconfig
  server.on("/connecttest.txt", [](AsyncWebServerRequest *request) {
    request->redirect("http://logout.net");
  }); // Connectivity test
  server.on("/redirect", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  }); // Generic redirect
  server.on("/generate_204", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  }); // Android captive portal check
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  }); // iOS captive portal check
  server.on("/ncsi.txt", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  }); // Windows connectivity check
  server.on("/success.txt", [](AsyncWebServerRequest *request) {
    request->send(200);
  }); // Success response
  server.on("/canonical.html", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  }); // Canonical redirect
  server.on("/favicon.ico", [](AsyncWebServerRequest *request) {
    request->send(404);
  }); // Favicon not found

  // Serve the main page
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response =
        request->beginResponse(200, "text/html", MAIN_page);
    response->addHeader("Cache-Control",
                        "public,max-age=31536000"); // Cache for 1 year
    request->send(response);
  });

  // Handle form submission to set time
  server.on("/SendForms", [](AsyncWebServerRequest *request) {
    String response = request->getParam(0)->value(); // Get form data
    request->send(200, "text/plain", response);      // Send response back
    deserializeJson(doc, response);                  // Parse JSON data

    int minute = doc["Minute"]; // Extract minute from JSON
    int hour = doc["Hour"];     // Extract hour from JSON

    // Set internal time based on form data (converting to seconds)
    internal_time.SetEpoch((Wrap(minute, 0, 59) * 60) +
                           (Wrap(hour, 0, 23) * 3600));
  });
}

/**
 * @brief Starts Wi-Fi Access Point
 * @param mode The Wi-Fi mode (e.g., WIFI_AP)
 * @param ssid The SSID for the access point
 * @param password The password for the access point
 * @param localIP The local IP address of the device
 * @param gatewayIP The gateway IP address
 */
void StartAP(const wifi_mode_t mode, const char *ssid, const char *password,
             const IPAddress &localIP, const IPAddress &gatewayIP) {
  WiFi.mode(mode); // Set Wi-Fi mode (e.g., WIFI_AP)
  WiFi.softAPConfig(localIP, gatewayIP, subnetMask); // Configure AP IP settings
  WiFi.softAP(ssid, password, WIFI_CHANNEL, 0, MAX_CLIENTS); // Start AP
  vTaskDelay(100 / portTICK_PERIOD_MS); // Small delay for stability
}

/**
 * @brief Setup DNS server for captive portal functionality
 * @param dnsServer The DNSServer instance to configure
 * @param localIP The local IP address of the device
 */
void SetupDNS(DNSServer &dnsServer, const IPAddress &localIP) {
  dnsServer.setTTL(3600);                    // Set DNS time-to-live to 1 hour
  dnsServer.start(53, "*", WiFi.softAPIP()); // Start DNS server on port 53
}

/**
 * --- UI Initialization ---
 */
BaseUi base_ui("title", "desc", &internal_time); // Base UI with header/footer
SettingsList settings_menu(1, 8, menuItems, &nav_info); // Settings menu screen

/**
 * @brief Setup function for the Arduino sketch
 */
void setup() {
  // StartAP(WIFI_AP, ssid, password, localIP, gatewayIP); // Start Wi-Fi AP
  // SetupDNS(dnsServer, localIP);                        // Initialize DNS
  // server

  // TEMP: connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  SetupServer(server, localIP); // Setup HTTP server
  server.begin();               // Start the server

  u8g2.begin();                     // Initialize OLED display
  Serial.begin(115200);             // Start serial communication at 115200 baud
  SPI.setClockDivider(CLOCK_SPEED); // Set SPI clock speed
  u8g2.setBusClock(CLOCK_SPEED);    // Set display bus clock
  delay(2000);                      // Delay for system stability

  // Register screens with navigation system
  nav_info.RegisterScreen(&settings_menu);
  nav_info.RegisterScreen(&time_menu);
  nav_info.RegisterScreen(&slider_menu);
  nav_info.SetCurrentScreen(&settings_menu); // Set initial screen

  Serial.println(WiFi.localIP()); // Print AP IP address for debugging
}

/**
 * @brief Main loop function for the Arduino sketch
 */
void loop() {
  u8g2.firstPage(); // Begin a new display page
  do {
    // dnsServer.processNextRequest();
    uint8_t input = getInput(); // Check for button input
    uint8_t input_result =
        nav_info.GetCurrentScreen()->HandleInput(input); // Handle input
    nav_info.GetCurrentScreen()->Draw(); // Draw the current screen

    // Switch screen if input result indicates a change
    if (input_result != NO_INPUT) {
      nav_info.SetScreenById(input_result);
    }

    internal_time.Tick(); // Update internal time
    base_ui.Draw();       // Draw base UI (header/footer)
  } while (u8g2.nextPage()); // Continue until all pages are drawn
}

/**
 * @file main.cpp
 * @brief Main Arduino sketch for a device with OLED display and web interface
 */

/**
 * --- Standard Arduino and Library Includes ---
 */
#include <Arduino.h>           // Core Arduino library for basic functions
#include <ArduinoJson.h>       // For JSON parsing and serialization
#include <ESPAsyncWebServer.h> // Ensure this is included (assumed from context)
#include <Preferences.h>       // For storing persistent data
#include <SPI.h>               // For SPI communication with display
#include <U8g2lib.h>           // For OLED display control
#include <Update.h>            // For OTA (Over-The-Air) updates
#include <time.h>              // For time-related functions

/**
 * --- Internal Project Headers ---
 */
#include <DebounceButton.h> // For debouncing button inputs
#include <Helpers.h>        // Helper functions for the project
#include <Icons.h>          // Icon definitions for UI
#include <Screens.h>        // Screen management classes
#include <Sensors.h>        // Sensor and relay data structs
#include <UiKit.h>          // UI toolkit for display
#include <WiFiInfo.h>       // WiFi information class
#include <WebServer.h>      // All webserver logic, to keep this file "tidy"

/**
 * --- Global Variables ---
 */
String current_screen = "Settings"; // Tracks the currently displayed screen
InternalTime internal_time;         // Manages internal time with user offset
NavInfo nav_info(0);         // Navigation info object initialized with ID 0
uint8_t debug_pin_value = 0; // Debug pin value for testing
SensorRelayManager manager;

TimeMenu time_menu(&internal_time, &nav_info, 2); // Time menu screen
SliderMenu slider_menu(&nav_info, 3);

/**
 * --- Menu Configuration ---
 */
const int kMenuNumItems = 8;        // Total number of menu items
const int KMenuMaxTitleLength = 22; // Max length for menu titles/descriptions

MenuItem menuItems[kMenuNumItems] = {
    MenuItem("Time", "Current Time", kClockIcon, 2),
    MenuItem("Slider Test", "Test ui slider", kPlaceholderIcon, 3),
    MenuItem("WiFi", "Manage WiFi / HotSpot", kPlaceholderIcon, 2),
    MenuItem("Fireworks", "desc 4", kPlaceholderIcon, 3),
    MenuItem("GPS Speed", "desc 5", kPlaceholderIcon, 4),
    MenuItem("Big Knob", "desc 6", kPlaceholderIcon, 5),
    MenuItem("Park Sensor", "desc 7", kPlaceholderIcon, 6),
    MenuItem("Turbo Gauge", "desc 8", kPlaceholderIcon, 7)};

/**
 * --- Button Objects for Debouncing ---
 */
DebounceButton upButton(BUTTON_UP_PIN);
DebounceButton downButton(BUTTON_DOWN_PIN);
DebounceButton selectButton(BUTTON_SELECT_PIN);

/**
 * --- Additional Global Variables ---
 */
int selected_menu_item = 0; // Index of the currently selected menu item
JsonDocument doc;           // JSON document for processing form data

/**
 * @brief Detects button input and returns the type
 * @return The type of input (UP, DOWN, SELECT, or NO_INPUT)
 */
uint8_t getInput() {
  if (upButton.isPressed())
    return UP;
  if (downButton.isPressed())
    return DOWN;
  if (selectButton.isPressed())
    return SELECT;
  return NO_INPUT;
}

/**
 * @brief Starts Wi-Fi Access Point
 */
void StartAP(const wifi_mode_t mode, const char *ssid, const char *password,
             const IPAddress &localIP, const IPAddress &gatewayIP) {
  WiFi.mode(mode);
  WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
  WiFi.softAP(ssid, password, WIFI_CHANNEL, 0, MAX_CLIENTS);
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

/**
 * @brief Setup DNS server for captive portal functionality
 */
void SetupDNS(DNSServer &dnsServer, const IPAddress &localIP) {
  dnsServer.setTTL(3600);
  dnsServer.start(53, "*", WiFi.softAPIP());
}

/**
 * --- UI Initialization ---
 */
BaseUi base_ui("title", "desc",
               &internal_time); // No clue what the warning is about..
SettingsList settings_menu(1, 8, menuItems, &nav_info);

/**
 * @brief Setup function for the Arduino sketch
 */
void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  SetupServer(server, localIP);
  server.begin();

  u8g2.begin();
  Serial.begin(115200);
  SPI.setClockDivider(CLOCK_SPEED);
  Serial.println(esp_reset_reason());
  u8g2.setBusClock(CLOCK_SPEED);
  delay(2000);

  nav_info.RegisterScreen(&settings_menu);
  nav_info.RegisterScreen(&time_menu);
  nav_info.RegisterScreen(&slider_menu);
  nav_info.SetCurrentScreen(&settings_menu);

  manager.LoadFromPreferences();
  for (int i = 0; i < manager.GetNumSensors(); i++) {
    Sensor *sensor = manager.sensors[i];
    if (sensor) {
      Serial.print("  Sensor ID: ");
      Serial.print(sensor->GetId());
      Serial.print(", Name: ");
      Serial.print(sensor->GetName());
      float value = sensor->GetValue();
      Serial.print(", Value: ");
      Serial.println(value);
    }
  }

  for (int i = 0; i < manager.GetNumRelays(); i++) {
    Relay *relay = manager.relays[i];
    if (relay) {
      Serial.println("  Relay ID: ");
      Serial.print(relay->GetId());
      Serial.println(" Relay Name: ");
      Serial.println(relay->GetName());
    }
  }

  Serial.println(WiFi.localIP());
}

// Defining variables here to keep them by their function (loop)
uint8_t input;
uint8_t input_result;

/**
 * @brief Main loop function for the Arduino sketch
 */

// Function to check if the button is pressed
bool buttonPressed() {
  if (upButton.isPressed() || downButton.isPressed() ||
      selectButton.isPressed()) {
    return true;
  } else {
    return false;
  }
}

unsigned long previousMillis = 0; // Store the last time the display was updated
const long interval = 5000;       // Interval at which to update (5 seconds)

/**
 * Main loop of the system.
 *
 * all the logic for updating the UI, handling inputs,
 * and managing sensor and relay states. It also includes a simple debug pin
 * value update mechanism.
 */
void loop() {
  static unsigned long lastUpdate = 0;
  const unsigned long updateInterval = 1000; // 1 second
  static bool displayDirty = false; // Flag to track if display needs updating

  input = getInput();

  // Check for input or time-based updates
  if (millis() - lastUpdate >= updateInterval || input != NO_INPUT) {
    lastUpdate = millis();
    displayDirty = true;

    // Handle screen navigation based on user input
    input_result = nav_info.GetCurrentScreen()->HandleInput(input);
    if (input_result != NO_INPUT) {
      nav_info.SetScreenById(input_result);
    }
    // Tick the internal time
    internal_time.Tick();

    // Update sensor values with debug pin value for testing
    bool sensorChanged = false;
    for (int i = 0; i < manager.GetNumSensors(); i++) {
      if (manager.sensors[i]) {
        float oldValue = manager.sensors[i]->GetValue();
        manager.sensors[i]->SetValue(debug_pin_value);
        if (oldValue != debug_pin_value) {
          sensorChanged = true;
        }
      }
    }

    // Evaluate and update relay states based on conditions
    bool relayChanged = false;
    for (int i = 0; i < manager.GetNumRelays(); i++) {
      if (manager.relays[i]) {
        Relay &relay = *manager.relays[i];
        bool shouldBeOn = evaluateRelayConditions(relay, manager);
        // Serial.println(shouldBeOn);
        if (shouldBeOn != relay.GetStatus()) {
          relay.SetStatus(shouldBeOn);
          // Serial.print("Relay updated: ");
          // Serial.println(relay.GetStatus());
          relayChanged = true;
        }
      }
    }
  }

  // Only render the screen when needed
  if (displayDirty) {
    u8g2.firstPage();
    do {
      nav_info.GetCurrentScreen()->Draw();
      base_ui.Draw();
    } while (u8g2.nextPage());
    displayDirty = false;
    // Serial.println("Display updated");
  }
}

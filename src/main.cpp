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
#include <index.h>          // HTML content for the web server

/**
 * --- Global Variables ---
 */
String current_screen = "Settings"; // Tracks the currently displayed screen
InternalTime internal_time;         // Manages internal time with user offset
NavInfo nav_info(0);         // Navigation info object initialized with ID 0
uint8_t debug_pin_value = 0; // Debug pin value for testing
SensorRelayManager manager;

/**
 * --- Hack Screen Class (Temporary/Debugging) ---
 * @brief A temporary screen class for debugging purposes
 */
class hack_screen : public Screen {
public:
  hack_screen() : Screen(0) {}
  void Draw() override { Serial.println("Hack Screen has been summoned"); }
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
 * @brief Setup HTTP server for captive portal functionality
 * @param server The AsyncWebServer instance to configure
 * @param localIP The local IP address of the device
 */
void SetupServer(AsyncWebServer &server, const IPAddress &localIP) {
  server.on("/wpad.dat",
            [](AsyncWebServerRequest *request) { request->send(404); });
  server.on("/connecttest.txt", [](AsyncWebServerRequest *request) {
    request->redirect("http://logout.net");
  });
  server.on("/redirect", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  });
  server.on("/generate_204", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  });
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  });
  server.on("/ncsi.txt", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  });
  server.on("/success.txt",
            [](AsyncWebServerRequest *request) { request->send(200); });
  server.on("/canonical.html", [](AsyncWebServerRequest *request) {
    request->redirect(localUrl);
  });
  server.on("/favicon.ico",
            [](AsyncWebServerRequest *request) { request->send(404); });

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response =
        request->beginResponse(200, "text/html", MAIN_page);
    response->addHeader("Cache-Control", "public,max-age=31536000");
    request->send(response);
  });

  server.on("/SendForms", [](AsyncWebServerRequest *request) {
    String response = request->getParam(0)->value();
    request->send(200, "text/plain", "All good");
    deserializeJson(doc, response);
    int minute = doc["Minute"];
    int hour = doc["Hour"];
    internal_time.SetEpoch((Wrap(minute, 0, 59) * 60) +
                           (Wrap(hour, 0, 23) * 3600));
  });

  server.on("/readADC", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Good");
  });

  // TEMP: Gonna put this in the main page, just for testing - template

  // Serve the firmware update page
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    const char *update_page = R"rawliteral(
  <!DOCTYPE html>
  <html lang='en'>
  <head>
      <meta charset='UTF-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <title>Firmware Update</title>
      <style>
          body { font-family: Arial, sans-serif; text-align: center; padding: 20px; }
          h1 { color: #333; }
          #status { margin: 20px 0; color: #666; }
          .button { padding: 10px 20px; background-color: #007BFF; color: white; border: none; cursor: pointer; }
          .button:hover { background-color: #0056b3; }
          input[type='file'] { margin: 10px 0; }
      </style>
  </head>
  <body>
      <h1>Firmware Update</h1>
      <form id='uploadForm' enctype='multipart/form-data'>
          <input type='file' name='file' accept='.bin' required>
          <br>
          <input type='submit' value='Update Firmware' class='button'>
      </form>
      <div id='status'>Select a .bin file to begin.</div>

      <script>
          document.getElementById('uploadForm').onsubmit = async function(e) {
              e.preventDefault();
              const fileInput = document.querySelector('input[type="file"]');
              const file = fileInput.files[0];
              if (!file) {
                  alert('Please select a firmware file!');
                  return;
              }

              const status = document.getElementById('status');
              status.textContent = 'Uploading...';

              const formData = new FormData();
              formData.append('file', file);

              try {
                  const response = await fetch('/update', {
                      method: 'POST',
                      body: formData
                  });
                  const text = await response.text();
                  status.textContent = text;
                  if (response.ok) {
                      status.style.color = '#28a745';
                      setTimeout(() => { status.textContent += ' Device is rebooting...'; }, 1000);
                  } else {
                      status.style.color = '#dc3545';
                  }
              } catch (error) {
                  status.textContent = 'Upload failed: ' + error.message;
                  status.style.color = '#dc3545';
              }
          };
      </script>
  </body>
  </html>
  )rawliteral";
    request->send(200, "text/html", update_page);
  });

  // Handle the firmware update
  server.on(
      "/update", HTTP_POST,
      [](AsyncWebServerRequest *request) {
        if (Update.hasError()) {
          request->send(500, "text/plain", "Update failed");
        } else {
          request->send(200, "text/plain", "Update successful. Rebooting...");
          delay(1000);
          ESP.restart();
        }
      },
      [](AsyncWebServerRequest *request, String filename, size_t index,
         uint8_t *data, size_t len, bool final) {
        if (!index) {
          if (!Update.begin(UPDATE_SIZE_UNKNOWN))
            return;
        }
        if (len) {
          Update.write(data, len);
        }
        if (final) {
          Update.end(true);
        }
      });

  // // Handle updating sensors, relays
  // server.on("/submit-sensors", HTTP_POST, [](AsyncWebServerRequest *request)
  // {
  //   if (request->hasParam("body", true)) {
  //     String jsonStr = request->getParam("body", true)->value();
  //     JsonDocument doc;
  //     DeserializationError error = deserializeJson(doc, jsonStr);

  //     if (error) {
  //       Serial.println("Invalid JSON received");
  //       request->send(400, "text/plain", "Invalid JSON");
  //       return;
  //     }

  //     // Parse and print sensors
  //     JsonArray sensorsArray = doc["sensors"];
  //     Serial.println("Received Sensors:");
  //     for (JsonObject sensorObj : sensorsArray) {
  //       Serial.print("  Sensor ID: ");
  //       Serial.print(sensorObj["id"].as<int>());
  //       Serial.print(", Name: ");
  //       Serial.print(sensorObj["name"].as<const char *>());
  //       Serial.print(", Pin: ");
  //       Serial.println(sensorObj["pin"].as<int>());
  //     }

  //     // Parse and print relays with their conditions
  //     JsonArray relaysArray = doc["relays"];
  //     Serial.println("Received Relays:");
  //     for (JsonObject relayObj : relaysArray) {
  //       Serial.print("  Relay ID: ");
  //       Serial.print(relayObj["id"].as<int>());
  //       Serial.print(", Name: ");
  //       Serial.print(relayObj["name"].as<const char *>());
  //       Serial.print(", Pin: ");
  //       Serial.println(relayObj["pin"].as<int>());

  //       // Print conditions for this relay
  //       JsonArray conditionsArray = relayObj["conditions"];
  //       if (conditionsArray.size() > 0) {
  //         Serial.println("    Conditions:");
  //         for (JsonObject conditionObj : conditionsArray) {
  //           Serial.print("      Condition ID: ");
  //           Serial.print(conditionObj["id"].as<int>());
  //           Serial.print(", Sensor: ");
  //           Serial.print(conditionObj["sensor"].as<int>());
  //           Serial.print(", SensorId: ");
  //           Serial.print(conditionObj["sensorId"].as<int>());
  //           Serial.print(", Operator: ");
  //           Serial.print(conditionObj["operator"].as<const char *>());
  //           Serial.print(", Type: ");
  //           Serial.println(conditionObj["type"].as<const char *>());
  //         }
  //       } else {
  //         Serial.println("    No conditions");
  //       }
  //     }

  //     request->send(200, "text/plain", "Sensors and relays received");
  //   } else {
  //     Serial.println("No data received in request");
  //     request->send(400, "text/plain", "No data received");
  //   }
  // });

  server.on("/submit-sensors", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("body", true)) {
      String jsonStr = request->getParam("body", true)->value();
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, jsonStr);

      if (error) {
        request->send(400, "text/plain", "Invalid JSON");
        return;
      }

      // Clear existing data
      manager = SensorRelayManager();

      // Parse and register sensors
      JsonArray sensorsArray = doc["sensors"];
      for (JsonObject sensorObj : sensorsArray) {
        uint8_t id = sensorObj["id"].as<int>();
        const char *name = sensorObj["name"].as<const char *>();
        uint8_t pin = sensorObj["pin"].as<int>();
        float value = 0.0; // Default value, updated in loop
        bool folded = true;
        Sensor *sensor = new Sensor(id, name, pin, value, folded);
        manager.RegisterSensor(sensor);
      }

      // Parse and register relays with conditions
      JsonArray relaysArray = doc["relays"];
      for (JsonObject relayObj : relaysArray) {
        uint8_t id = relayObj["id"].as<int>();
        const char *name = relayObj["name"].as<const char *>();
        uint8_t pin = relayObj["pin"].as<int>();
        bool status = false;
        bool folded = false;
        Relay *relay = new Relay(id, name, pin, status, folded);

        // Register conditions
        JsonArray conditionsArray = relayObj["conditions"];
        for (JsonObject conditionObj : conditionsArray) {
          uint8_t conditionId = conditionObj["id"].as<int>();
          uint8_t sensor = conditionObj["sensor"].as<int>();
          uint8_t sensorId = conditionObj["sensorId"].as<int>();
          const char *op = conditionObj["operator"].as<const char *>();
          float value = conditionObj["value"].as<float>();
          const char *type = conditionObj["type"].as<const char *>();
          Condition *condition =
              new Condition(sensor, sensorId, op, value, conditionId, type);
          relay->AddCondition(condition);
        }

        manager.RegisterRelay(relay);
        pinMode(pin, OUTPUT);
        digitalWrite(pin, status ? HIGH : LOW);
      }

      request->send(200, "text/plain", "Sensors and relays updated");
    } else {
      request->send(400, "text/plain", "No data received");
    }
  });

  server.on("/update-debug-pin", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("body", true)) {
      debug_pin_value = request->getParam("body", true)->value().toInt();
    }
  });
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
  static bool displayDirty = true; // Flag to track if display needs updating

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
        Serial.println(shouldBeOn);
        if (shouldBeOn != relay.GetStatus()) {
          relay.SetStatus(shouldBeOn);
          Serial.print("Relay updated: ");
          Serial.println(relay.GetStatus());
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
    Serial.println("Display updated");
  }
}

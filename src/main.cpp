// Standard Arduino and library includes
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Update.h>
#include <time.h>

// Internal project headers
#include <DebounceButton.h>
#include <Helpers.h>
#include <Screens.h>
#include <UiKit.h>
#include <index.h>
#include <Icons.h>

// Global variables
String current_screen = "Settings"; // Current screen being displayed
InternalTime internal_time;         // Manages internal time with user offset
NavInfo nav_info(0);                // NavInfo object for navigation information

// HACK: lol
class hack_screen : public Screen {
public:
  hack_screen() : Screen(0) {}

  void Draw() override { Serial.println("Hack Screen hast been summoned"); }
  uint8_t HandleInput(uint8_t input) override {
    Serial.println("Hack Screen input: " + String(input));
    return 0;
  }
};

hack_screen new_hack_screen;
TimeMenu time_menu(&internal_time, &nav_info, 2);

// Menu Configuration
const int kMenuNumItems = 2; // Number of menu items
const int KMenuMaxTitleLength =
    22; // Maximum length of menu titles/descriptions

MenuItem menuItems[kMenuNumItems] = {
    MenuItem("Time", "Current Time", kClockIcon, 2, &new_hack_screen),
    MenuItem("Slider Test", "Test ui slider", kPlaceholderIcon, 3,
             &new_hack_screen)
    // MenuItem("WiFi", "Manage WiFi / HotSpot", kPlaceholderIcon, 2,
    //          &new_hack_screen),
    // MenuItem("Fireworks", "desc 4", kPlaceholderIcon, 3, &new_hack_screen),
    // MenuItem("GPS Speed", "desc 5", kPlaceholderIcon, 4, &new_hack_screen),
    // MenuItem("Big Knob", "desc 6", kPlaceholderIcon, 5, &new_hack_screen),
    // MenuItem("Park Sensor", "desc 7", kPlaceholderIcon, 6, &new_hack_screen),
    // MenuItem("Turbo Gauge", "desc 8", kPlaceholderIcon, 7, &new_hack_screen)
  };

// Button objects for debouncing
DebounceButton upButton(BUTTON_UP_PIN);         // UP button
DebounceButton downButton(BUTTON_DOWN_PIN);     // DOWN button
DebounceButton selectButton(BUTTON_SELECT_PIN); // SELECT button

int current_setting_unit =
    0; // Tracks the unit being changed (minutes/hours/done)
int selected_menu_item = 0; // Tracks the selected item in the menu

JsonDocument doc; // For processing JSON data from forms

// Function to get the current input from buttons
uint8_t getInput() {
  if (upButton.isPressed()) {
    return UP;
  }
  if (downButton.isPressed()) {
    return DOWN;
  }
  if (selectButton.isPressed()) {
    return SELECT;
  }
  return NO_INPUT;
}

// Set up the HTTP server with various endpoints
void SetupServer(AsyncWebServer &server, const IPAddress &localIP) {
  // Handle specific URLs for captive portal behavior
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
  // Handle favicon request with 404
  server.on("/favicon.ico",
            [](AsyncWebServerRequest *request) { request->send(404); });

  // Serve the main page
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response =
        request->beginResponse(200, "text/html", MAIN_page);
    response->addHeader("Cache-Control", "public,max-age=31536000");
    request->send(response);
  });

  // Handle form submissions to set time
  server.on("/SendForms", [](AsyncWebServerRequest *request) {
    String response = request->getParam(0)->value();
    request->send(200, "text/plain", response);
    deserializeJson(doc, response);

    int minute = doc["Minute"];
    int hour = doc["Hour"];

    // Set the internal time based on form data
    internal_time.set_epoch((Wrap(minute, 0, 59) * 60) +
                            (Wrap(hour, 0, 23) * 3600));
  });
}

// Start the Wi-Fi access point
void StartAP(const wifi_mode_t mode, const char *ssid, const char *password,
             const IPAddress &localIP, const IPAddress &gatewayIP) {
  WiFi.mode(mode);
  WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
  WiFi.softAP(ssid, password, WIFI_CHANNEL, 0, MAX_CLIENTS);
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

// Set up the DNS server
void SetupDNS(DNSServer &dnsServer, const IPAddress &localIP) {
  dnsServer.setTTL(3600);
  dnsServer.start(53, "*", WiFi.softAPIP());
}

// Initialize the base UI
BaseUi base_ui("title", "desc", &internal_time);
SettingsList settings_menu(1, 8, menuItems, &nav_info);

void setup() {
  // Initialize Wi-Fi as an access point
  StartAP(WIFI_AP, ssid, password, localIP,
          gatewayIP); // Changed to WIFI_AP for access point mode

  // Set up DNS server
  SetupDNS(dnsServer, localIP);

  // Set up the HTTP server
  SetupServer(server, localIP);
  server.begin();

  // Initialize the display
  u8g2.begin();

  // Initialize serial communication
  Serial.begin(115200);

  // Set SPI clock speed
  SPI.setClockDivider(CLOCK_SPEED);
  u8g2.setBusClock(CLOCK_SPEED);

  // Delay for stability
  delay(2000);

  nav_info.RegisterScreen(&settings_menu);
  nav_info.RegisterScreen(&time_menu);
  nav_info.SetCurrentScreen(&settings_menu);

  // Print the local IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Start a new page for the display
  u8g2.firstPage();
  do {
    // settings_menu.HandleInput(getInput());
    uint8_t input = getInput();
    // settings_menu.Draw();
    nav_info.GetCurrentScreen()->Draw();
    uint8_t input_result = nav_info.GetCurrentScreen()->HandleInput(input);
    // Get the current input from buttons
    if (input != NO_INPUT) {
      // Print the input for debugging
      Serial.println("Input: " + String(input));
      Serial.println(input_result);
    }

    if (input_result != NO_INPUT) {
      nav_info.SetScreenById(input_result);
      nav_info.test();
      Serial.println(nav_info.GetCurrentScreenId());
      Serial.println("Selected: " + String(input_result));
    }

    // Update the internal time
    internal_time.tick();
    // Draw the header/footer UI on the display
    base_ui.Draw();
    // settings_menu.Draw();  // Uncomment if needed
  } while (u8g2.nextPage());
}

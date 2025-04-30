#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Update.h>
#include <time.h>

// Internal Headers
#include <DebounceButton.h>
#include <Helpers.h>
#include <UiKit.h>
#include <index.h>

// Current screen
String current_screen = "Settings";

// Keep track of time + user offset
InternalTime internal_time;
// Buttons
DebounceButton upButton(BUTTON_UP_PIN);         // Pin for UP button
DebounceButton downButton(BUTTON_DOWN_PIN);     // Pin for DOWN button
DebounceButton selectButton(BUTTON_SELECT_PIN); // Pin for SELECT button

int current_setting_unit = 0; // keep track of the unit being changed from the
                              // menu (minutes/hours/done)

// Keep track of which item is selected in the menu
int selected_menu_item = 0;

// form processing variables
JsonDocument doc;

// Setup the HTTP server
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

  // 404 the favicon
  server.on("/favicon.ico",
            [](AsyncWebServerRequest *request) { request->send(404); });

  // Serve the main page
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response =
        request->beginResponse(200, "text/html", MAIN_page);
    response->addHeader("Cache-Control", "public,max-age=31536000");
    request->send(response);
  });

  server.on("/SendForms", [](AsyncWebServerRequest *request) {
    String response = request->getParam(0)->value();
    request->send(200, "text/plain", response);
    deserializeJson(doc, response);

    int minute = doc["Minute"];
    int hour = doc["Hour"];

    internal_time.set_epoch((Wrap(minute, 0, 59) * 60) +
                            (Wrap(hour, 0, 23) * 3600));
  });
}

void StartAP(const wifi_mode_t mode, const char *ssid, const char *password,
             const IPAddress &localIP, const IPAddress &gatewayIP) {
  WiFi.mode(mode);
  WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
  WiFi.softAP(ssid, password, WIFI_CHANNEL, 0, MAX_CLIENTS);
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

// Setup the DNS server so that localUrl can be resolved
void SetupDNS(DNSServer &dnsServer, const IPAddress &localIP) {
  dnsServer.setTTL(3600);
  dnsServer.start(53, "*", WiFi.softAPIP());
}

// Initialize everything
// DEBUG
BaseUi ui("Main", "Debug", &internal_time);

void setup() {

  // Set up the WiFi
  StartAP(WIFI_STA, ssid, password, localIP, gatewayIP);
  SetupDNS(dnsServer, localIP);

  SetupServer(server, localIP);
  server.begin();

  u8g2.begin();
  Serial.begin(115200);
  SPI.setClockDivider(CLOCK_SPEED);
  u8g2.setBusClock(CLOCK_SPEED);

  delay(2000);
  Serial.println(WiFi.localIP());
}

// Debug test
// ListMenu list_menu(SettingsMenus);

// Return current input
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

void loop() {
  u8g2.firstPage();
  do {
    uint8_t input = getInput();
    if (input != NO_INPUT) {
      Serial.println("Input: " + String(input));
    };
    internal_time.tick();
    ui.Draw();
    // list_menu.Draw();
  } while (u8g2.nextPage());
}
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
#include <index.h>

// Button states
int button_down_clicked = 0;
int button_select_clicked = 0;
int button_up_clicked = 0;

// Current screen
String current_screen = "Settings";

// Keep track of time + user offset
InternalTime internal_time;
DebounceButton button(25);

int current_setting_unit = 0; // keep track of the unit being changed from the
                              // menu (minutes/hours/done)

// Keep track of which item is selected in the menu
int selected_menu_item = 0;

// The http processing
JsonDocument doc;

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

void setup() {
  Serial.begin(115200);
}

void loop() {
  if (button.isPressed()) {
    Serial.println("Button pressed");
  }
}
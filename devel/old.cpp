#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Update.h>
#include <time.h>

// Internal Headers
#include <helpers.h>
#include <index.h>

// Button states
int button_down_clicked = 0;
int button_select_clicked = 0;
int button_up_clicked = 0;

// Current screen
String current_screen = "Settings";

// Keep track of time + user offset
long offset_time = 0;
int updated_hour = 0;
int updated_minute = 0;
int updated_offset = 0;

int current_setting_unit = 0; // keep track of the unit being changed from the
                              // menu (minutes/hours/done)

// Current realtime clock values
int current_second = 0;
int current_minute = 0;
int current_hour = 0;

// Keep track of which item is selected in the menu
int selected_menu_item = 0;

// AP config
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

// handle the upload of the firmware
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index,
                  uint8_t *data, size_t len, bool final) {
  // handle upload and update
  if (!index) {
    Serial.printf("Update: %s\n", filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // start with max available size
      Update.printError(Serial);
    }
  }
  /* flashing firmware to ESP*/
  if (len) {
    Update.write(data, len);
  }

  if (final) {
    if (Update.end(true)) { // true to set the size to the current progress
      Serial.printf("Update Success: %ub written\nRebooting...\n", index + len);
    } else {
      Update.printError(Serial);
    }
  }
  // alternative approach
  // https://github.com/me-no-dev/ESPAsyncWebServer/issues/542#issuecomment-508489206
}

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

  // 404 favicon
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
    Serial.println("Responded");
    Serial.println(response);
    deserializeJson(doc, response);

    int minute = doc["Minute"];
    int hour = doc["Hour"];

    updated_minute = Wrap(minute, 0, 59);
    updated_hour = Wrap(hour, 0, 23);
    updated_offset = time(&now);

    Serial.println(hour);
  });

  server.on(
      "/update-firmware", HTTP_POST,
      [](AsyncWebServerRequest *request) {
        if (!Update.hasError()) {
          AsyncWebServerResponse *response =
              request->beginResponse(200, "text/plain", "OK");
          response->addHeader("Connection", "close");
          request->send(response);
          ESP.restart();
        } else {
          AsyncWebServerResponse *response =
              request->beginResponse(500, "text/plain", "ERROR");
          response->addHeader("Connection", "close");
          request->send(response);
        }
      },
      handleUpload);
}

// UI functions
// Draw simple ui elements (titlebar, footer)
void drawUI(const char *title, const char *description) {
  u8g2.setFontMode(1);
  u8g2.drawBox(0, 0, SCREEN_WIDTH, 10);
  u8g2.setDrawColor(2);
  u8g2.setFont(u8g_font_baby);
  u8g2.drawStr(0, 7, title);
  char time_str[20];
  sprintf(time_str, "%02d:%02d", current_hour, current_minute);
  u8g2.drawStr(103, 7, time_str);

  u8g2.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10);
  u8g2.drawStr(0, SCREEN_HEIGHT - 2, description);
}
// Draw individual menu items
void drawMenuItem(int position, const char *label,
                  const unsigned char *icon = kSquareIcon) {
  int textY = (position == 1) ? 28 : 44;
  int iconY = (position == 1) ? 15 : 33;

  u8g2.drawStr(25, textY, label);
  u8g2.drawBitmap(4, iconY, 16 / 8, 16, icon);
}
// Draw slider
void drawSlider(int smallSliderValue, int bigSliderValue) {
  u8g2.drawLine(10, SCREEN_HEIGHT - 20, SCREEN_WIDTH - 10,
                SCREEN_HEIGHT - 20); // horizontal line across whole 128 screen

  // Line end.. border? idk - looks nice
  u8g2.drawLine(SCREEN_WIDTH - 9, SCREEN_HEIGHT - 22, SCREEN_WIDTH - 9,
                SCREEN_HEIGHT - 18);
  u8g2.drawLine(9, SCREEN_HEIGHT - 22, 9, SCREEN_HEIGHT - 18);

  // Draw the indicators
  int smallIndicatorX = (map(smallSliderValue, 0, 100, 10, SCREEN_WIDTH - 10));
  int bigIndicatorX = (map(bigSliderValue, 0, 100, 10, SCREEN_WIDTH - 10));

  u8g2.drawLine(smallIndicatorX, SCREEN_HEIGHT - 23, smallIndicatorX,
                SCREEN_HEIGHT - 17);
  u8g2.drawLine(bigIndicatorX, SCREEN_HEIGHT - 23, bigIndicatorX,
                SCREEN_HEIGHT - 17);

  // Convert ints into char
  char smallSliderValueChar[12];
  char bigSliderValueChar[12];

  // Draw the text (above indicators)
  if ((bigSliderValue - smallSliderValue) > 11) {
    sprintf(smallSliderValueChar, "%d", smallSliderValue);
    sprintf(bigSliderValueChar, "%d", bigSliderValue);

    u8g2.drawStr(smallIndicatorX - 4, SCREEN_HEIGHT - 25, smallSliderValueChar);
    u8g2.drawStr(bigIndicatorX - 4, SCREEN_HEIGHT - 25, bigSliderValueChar);
  } else {
    // Use the same variable to keep memory use low
    sprintf(smallSliderValueChar, "%d - %d", smallSliderValue, bigSliderValue);

    u8g2.drawStr(Clamp(smallIndicatorX - 4, 10, 95), 38, smallSliderValueChar);
  }
}
// Update the offset time and all derivative values
void updateTimes() {
  offset_time =
      time(&now) - updated_offset + updated_hour * 3600 + updated_minute * 60;
  current_second = WrapSeconds(offset_time);
  current_minute = SecondsToMinutes(offset_time);
  current_hour = SecondsToHours(offset_time);
}

int loopTick = 0;
bool firstTick = true;
// Reset the button pressed values for the conditions
void resetButtons() {
  if ((digitalRead(BUTTON_UP_PIN) == HIGH)) {
    button_up_clicked = 0;
  }
  if ((digitalRead(BUTTON_DOWN_PIN) == HIGH)) {
    button_down_clicked = 0;
  }
  if ((digitalRead(BUTTON_SELECT_PIN) == HIGH)) {
    button_select_clicked = 0;
  }

  // If any of the buttons are pressed start counting the loop
  if ((digitalRead(BUTTON_UP_PIN) == LOW) ||
      ((digitalRead(BUTTON_DOWN_PIN) == LOW)) ||
      ((digitalRead(BUTTON_SELECT_PIN) == LOW))) {
    loopTick++;
    if ((loopTick > 85 && firstTick) || (loopTick > 25 && !firstTick)) {
      loopTick = 0;
      firstTick = false;
      button_up_clicked = 0;
      button_down_clicked = 0;
      button_select_clicked = 0;
    }
  } else {
    loopTick = 0;
    firstTick = true;
  }
}

// All of ze' menus
void drawSettingsMenu() {
  drawUI("Settings", menu_item_descriptions[selected_menu_item]);
  // Serial.println(selected_menu_item);
  // Serial.println(menu_items[selected_menu_item]);

  // Navigation
  if (UP_CONDITION) {
    ++selected_menu_item;
    button_up_clicked = 1;
    preferences.putUInt("smi", selected_menu_item);
  } else if (DOWN_CONDITION) {
    --selected_menu_item;
    button_down_clicked = 1;
    preferences.putUInt("smi", selected_menu_item);
  } else if (SELECT_CONDITION) {
    button_select_clicked = 1;
    current_screen = menu_items[selected_menu_item];
    Serial.println(menu_items[selected_menu_item]);
    Serial.println(current_screen);

    // Reset the time settings if the user selects the Time menu item
    current_setting_unit = 0;
    if (strcmp(menu_items[selected_menu_item], "Time") == 0) {
      updated_hour = 0;
      updated_minute = 0;
    }
  }
  selected_menu_item = Wrap(selected_menu_item, 0, kMenuNumItems - 1);

  // Draw the menu items
  u8g2.setFont(u8g_font_7x13B);
  drawMenuItem(1, menu_items[selected_menu_item]);

  if (selected_menu_item < kMenuNumItems - 1) {
    u8g2.setFont(u8g_font_5x8);
    drawMenuItem(2, menu_items[selected_menu_item + 1]);
  }
}

void drawTimeMenu() {
  drawUI("Time", "Yeah.. Its the time");

  // The buffers for information that will go to the screen
  char time_str[10];
  char seconds_str[4];

  // Buffer appropriate time string to the screen (preview/actual)
  if (current_setting_unit < 2) {
    sprintf(time_str, "%02d:%02d", updated_hour, updated_minute);
    sprintf(seconds_str, "%02d", 00);
  } else {
    sprintf(time_str, "%02d:%02d", current_hour, current_minute);
    sprintf(seconds_str, "%02d", current_second);
  }

  u8g2.setFont(u8g_font_10x20r);

  // Draw the indicator boxes
  switch (current_setting_unit) {
  case 0:
    u8g2.drawBox(65, 16, 20, 19);
    break;
  case 1:
    u8g2.drawBox(35, 16, 20, 19);
    break;
  }

  // Draw the time and second
  u8g2.drawStr(35, 32, time_str);
  u8g2.setFont(u8g_font_baby);
  u8g2.drawStr(85, 32, seconds_str);

  // Button handling
  if (UP_CONDITION) {
    button_up_clicked = 1;

    // Add to the minute/hour
    switch (current_setting_unit) {
    case 0:
      updated_minute = Wrap(updated_minute + 1, 0, 59);
      break;
    case 1:
      updated_hour = Wrap(updated_hour + 1, 0, 23);
      break;
    }
  } else if (DOWN_CONDITION) {
    button_down_clicked = 1;

    // Subtract from the minute/hour
    switch (current_setting_unit) {
    case 0:
      updated_minute = Wrap(updated_minute - 1, 0, 59);
      break;
    case 1:
      updated_hour = Wrap(updated_hour - 1, 0, 23);
      break;
    }
  } else if (SELECT_CONDITION) {
    button_select_clicked = 1;
    current_setting_unit++;

    if (current_setting_unit == 2) {
      updated_offset = time(&now);
    } else if (current_setting_unit == 3) {
      current_screen = "Settings";
    }
  }
}

// Vars for slider test menu
int sliderMin = 0;
int sliderMax = 100;
int smallSliderValue = 30; // Initial value of the slider
int bigSliderValue = 70;   // Initial value of the slider

void drawSliderTestMenu() {
  drawUI("Slider Test", "Test of slider... yeah.");

  // Super simple, add and subtract from the values from user info
  if (UP_CONDITION) {
    button_up_clicked = 1;
    if (current_setting_unit == 0 && smallSliderValue < sliderMax - 1) {
      smallSliderValue++;
    } else if (current_setting_unit == 1 && bigSliderValue < sliderMax) {
      bigSliderValue++;
    }
  } else if (DOWN_CONDITION) {
    button_down_clicked = 1;
    if (current_setting_unit == 0 && smallSliderValue > sliderMin) {
      smallSliderValue--;
    } else if (current_setting_unit == 1 && bigSliderValue > sliderMin + 1) {
      bigSliderValue--;
    }
  } else if (SELECT_CONDITION) {
    button_select_clicked = 1;
    current_setting_unit++;
  }

  // Ensure they stay in range/in order with each other
  if ((smallSliderValue >= bigSliderValue) && current_setting_unit == 1) {
    smallSliderValue = bigSliderValue - 1;
    if ((smallSliderValue >= bigSliderValue) && current_setting_unit == 0) {
      bigSliderValue = smallSliderValue + 1;
    }
  }
  if (bigSliderValue <= smallSliderValue) {
    bigSliderValue = smallSliderValue + 1;
  }

  // Return to settings
  if (current_setting_unit > 1) {
    current_screen = "Settings";
  }

  // DEBUG: Render text with stats
  // char sliderText[30];
  // sprintf(sliderText, "Small: %d, Big: %d, Unit: %d", smallSliderValue,
  // bigSliderValue, current_setting_unit); Serial.println(sliderText);

  // Draw the slider
  drawSlider(smallSliderValue, bigSliderValue);
  // u8g2.drawStr(0, 40, sliderText);
}

bool isWifi = false;

void drawWifiMenu() {
  drawUI("WiFi", "IoT Settings");

  u8g2.setFont(u8g_font_7x13B);
  u8g2.drawStr(24, 25, "Select Mode");

  u8g2.setFont(u8g_font_5x8);
  if (isWifi) {
    u8g2.drawStr(38, 35, "Mode: WiFi");
  } else {
    u8g2.drawStr(32, 35, "Mode: Access Point");
  }

  u8g2.setFont(u8g_font_baby);
  u8g2.drawStr(9, 52, "Configure WiFi through AP");
}

// Setup the screen, time and buttons
void setup() {
  Serial.begin(115200);
  SPI.setClockDivider(CLOCK_SPEED);
  u8g2.setBusClock(CLOCK_SPEED);

  delay(2000); // Let the SPI clock settle before drawing anything.

  // Load the persistent storage and set selected item
  preferences.begin("settings", false);
  selected_menu_item = preferences.getUInt("smi");

  // Pull up buttons so they can be read
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);

  // Set up the WiFi
  StartAP(WIFI_STA, ssid, password, localIP, gatewayIP);
  SetupDNS(dnsServer, localIP);

  SetupServer(server, localIP);
  server.begin();

  // Finally, initialize the screen.
  u8g2.begin();
}

void loop() {
  u8g2.firstPage();
  do {
    // Serial.println(WiFi.localIP());
    dnsServer.processNextRequest();
    updateTimes();
    resetButtons();

    // Serial.println("Current Screen" + current_screen);
    // Serial.println(menu_items[selected_menu_item]);

    // Draw each menup
    if (current_screen == "Settings") {
      drawSettingsMenu();
    } else if (current_screen == "Time") {
      drawTimeMenu();
    } else if (current_screen == "Slider Test") {
      drawSliderTestMenu();
    } else if (current_screen == "WiFi") {
      drawWifiMenu();
    }
  } while (u8g2.nextPage());
}
enum ButtonState { IDLE, PRESSED, DEBOUNCING };

ButtonState state = IDLE;

void DebounceButton::handleInterrupt() {
  if (initialized && state == IDLE) {
    if (digitalRead(buttonPin) == LOW) {
      state = PRESSED;
      elapseTimer = millis();
    }
  }
}

bool DebounceButton::check() {
  int buttonReading = digitalRead(buttonPin);
  switch (state) {
    case IDLE:
      break;
    case PRESSED:
      if (buttonReading == LOW && millis() - elapseTimer >= debounceTime) {
        state = DEBOUNCING;
      }
      break;
    case DEBOUNCING:
      if (buttonReading == HIGH && millis() - elapseTimer >= debounceTime) {
        state = IDLE;
        return true; // Button press detected
      }
      break;
  }
  return false;
}

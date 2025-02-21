#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Preferences.h>
#include <time.h>

// internal
#include <helpers.h>

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

int current_time_unit = 0; // keep track of the unit being changed from the menu (minutes/hours/done)

// Current realtime clock values
int current_second = 0;
int current_minute = 0;
int current_hour = 0;

// Keep track of which item is selected in the menu
int selected_menu_item = 0;

// UI functions
// Draw simple ui elements (titlebar, footer)
void drawUI(const char *title, const char *description)
{
  u8g2.setFontMode(1);
  u8g2.drawBox(0, 0, SCREEN_WIDTH, 10);
  u8g2.setDrawColor(2);
  u8g2.setFont(u8g_font_baby);
  u8g2.drawStr(0, 7, title);

  u8g2.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10);
  u8g2.drawStr(0, SCREEN_HEIGHT - 2, description);
}
// Draw individual menu items
void drawMenuItem(int position, const char *label, const unsigned char* icon = kSquareIcon)
{
  int textY = (position == 1) ? 28 : 44;
  int iconY = (position == 1) ? 15 : 33;

  u8g2.drawStr(25, textY, label);
  u8g2.drawBitmap(4, iconY, 16 / 8, 16, icon);
}

// Update the offset time and all derivative values
void updateTimes() {
	offset_time = time(&now) - updated_offset + updated_hour * 3600 + updated_minute * 60;
	current_second = WrapSeconds(offset_time);
	current_minute = SecondsToMinutes(offset_time);
	current_hour = SecondsToHours(offset_time);
}

// Reset the button pressed values for the conditions
void resetButtons() {
	if ((digitalRead(BUTTON_UP_PIN) == HIGH))
	{
		button_up_clicked = 0;
	}
	if ((digitalRead(BUTTON_DOWN_PIN) == HIGH))
	{
		button_down_clicked = 0;
	}
	if ((digitalRead(BUTTON_SELECT_PIN) == HIGH))
	{
		button_select_clicked = 0;
	}
}

// All of ze' menus
void drawSettingsMenu() {
	drawUI("Settings", menu_item_descriptions[selected_menu_item]);

	// Navigation
	if (UP_CONDITION) {
		++selected_menu_item;
		button_up_clicked = 1;
		preferences.putUInt("selected_menu_item", selected_menu_item);
	} else if (DOWN_CONDITION) {
		--selected_menu_item;
		button_down_clicked = 1;
		preferences.putUInt("selected_menu_item", selected_menu_item);
	} else if (SELECT_CONDITION) {
		button_select_clicked = 1;
		current_screen = menu_items[selected_menu_item];
	}
	selected_menu_item = Wrap(selected_menu_item, 0, kMenuNumItems - 1);

	// Draw the menu items
	u8g2.setFont(u8g_font_7x13B);
	drawMenuItem(1, menu_items[selected_menu_item]);

	if (selected_menu_item < kMenuNumItems - 1)
	{
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
	if (current_time_unit < 2) {
		sprintf(time_str, "%02d:%02d", updated_hour, updated_minute);
		sprintf(seconds_str, "%02d", 00);
	}
	else {
		sprintf(time_str, "%02d:%02d", current_hour, current_minute);
		sprintf(seconds_str, "%02d", current_second);
	}

	u8g2.setFont(u8g_font_10x20r);

	// Draw the indicator boxes
	switch (current_time_unit) {
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
	switch (current_time_unit)
	{
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
		switch (current_time_unit)
		{
			case 0:
				updated_minute = Wrap(updated_minute - 1, 0, 59);
				break;
			case 1:
				updated_hour = Wrap(updated_hour - 1, 0, 23);
				break;
		}
	} else if (SELECT_CONDITION) {
		button_select_clicked = 1;
		current_time_unit++;

		if (current_time_unit == 2) {
			updated_offset = time(&now);
		}
	}
}

// Setup the screen, time and buttons
void setup() {
	Serial.begin(115200);
  SPI.setClockDivider(CLOCK_SPEED);
  u8g2.setBusClock(CLOCK_SPEED);

	delay(2000); // Let the SPI clock settle before drawing anything.

	// Load the persistent storage and set selected item
	preferences.begin("settings", false);
  selected_menu_item = preferences.getUInt("selected_menu_item");

	// Pull up buttons so they can be read
	pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);

	// Finally, initialize the screen.
  u8g2.begin();
	}

void loop() {
	u8g2.firstPage();
	do {
		updateTimes();
		resetButtons();

		// Draw each menu
		if (current_screen == "Settings") {
			drawSettingsMenu();
		} else if (current_screen == "Time")
		{
			drawTimeMenu();
		}

	} while (u8g2.nextPage());
}
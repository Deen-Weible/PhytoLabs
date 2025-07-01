// screens.h
/**
 * @file Screens.h
 * @brief Header file containing definitions for UI-related classes and
 * functions
 */

#ifndef SCREENS_H
#define SCREENS_H

#include <Helpers.h>
#include <UiKit.h>

/**
 * @class TimeMenu
 * @brief Class representing a time settings menu
 * @details This class provides functionality to adjust time settings (hours,
 * minutes, seconds) and displays the current time on the screen.
 */
class TimeMenu : public Screen {

public:
  /**
   * @brief Constructor for TimeMenu
   * @param time Pointer to InternalTime object
   * @param nav_info Navigation information pointer
   * @param screen_id Unique identifier for the screen
   */
  TimeMenu(InternalTime *time, NavInfo *nav_info, uint8_t screen_id)
      : time(time), nav_info(nav_info), Screen(screen_id) {};

  /**
   * @brief Draw the time menu on the display
   * @details This method renders the current time and time settings interface.
   * It displays the time in a formatted string and draws the indicator box for
   * setting units.
   */
  void Draw() override {
    // Buffer appropriate time string to the screen (preview/actual)
    if (current_setting_unit < 2) {
      sprintf(time_str, "%02d:%02d", updated_hour, updated_minute);
      sprintf(seconds_str, "%02d", 00);
    } else {
      sprintf(time_str, "%02d:%02d", time->GetHour(), time->GetMinute());
      sprintf(seconds_str, "%02d", time->GetSecond());
    }

    u8g2.setFont(u8g_font_10x20r);

    // Draw the indicator box
    if (current_setting_unit < 2) {
      u8g2.drawBox(65 - (current_setting_unit * 30), 16, 20, 19);
    }

    // Draw the time and second
    u8g2.drawStr(35, 32, time_str);
    u8g2.setFont(u8g_font_baby);
    u8g2.drawStr(85, 32, seconds_str);
  };

  /**
   * @brief Handle user input for the time menu
   * @param input Input value from the user (SELECT, UP, DOWN)
   * @return 1 if the screen should be redrawn, 0 otherwise
   */
  uint8_t HandleInput(uint8_t input) override {
    if (input == SELECT) {
      current_setting_unit += 1;
    }

    if (!current_setting_unit) {
      // Handle minute setting
      if (input == UP) {
        updated_minute = Wrap(updated_minute + 1, 0, 59);
      } else if (input == DOWN) {
        updated_minute = Wrap(updated_minute - 1, 0, 59);
      }
    } else if (current_setting_unit == 1) {
      // Handle hour setting
      if (input == UP) {
        updated_hour = Wrap(updated_hour + 1, 0, 23);
      } else if (input == DOWN) {
        updated_hour = Wrap(updated_hour - 1, 0, 23);
      }
    } else if (current_setting_unit == 2 && input == SELECT) {
      // Save the new time
      time->ResetRTC();
      time->SetCurrentTime(updated_hour, updated_minute);
      // reset the counter
      current_setting_unit = 0;

      // HACK: temp
      Serial.println("Time menu: Third input");
      return 1;
    }

    return 0;
  };

private:
  char time_str[6];             /**< Time string buffer for hours and minutes */
  char seconds_str[3];          /**< Time string buffer for seconds */
  uint8_t updated_hour = 00;    /**< Updated hour value for time setting */
  uint8_t updated_minute = 00;  /**< Updated minute value for time setting */
  uint8_t current_setting_unit; /**< Current setting unit (0: minutes, 1: hours,
                                   2: seconds) */

  InternalTime *time; /**< Pointer to internal time object */
  NavInfo *nav_info;  /**< Navigation information pointer */
};

class SliderMenu : public Screen {
public:
  /**
   * @brief Constructor for SliderMenu
   * @param nav_info Navigation information pointer
   * @param screen_id Unique identifier for the screen
   * @param min Minimum value for slider (default 0)
   * @param max Maximum value for slider (default 100)
   */
  SliderMenu(NavInfo *nav_info, uint8_t screen_id, int min = 0, int max = 100)
      : Screen(screen_id), nav_info(nav_info), min(min), max(max) {}

  /**
   * @brief Draw the slider menu on the display
   * @details This method renders a horizontal slider track with indicators for
   * the current values of min and max. It displays the values above the
   * sliders.
   */
  void Draw() override {
    // Constants for slider layout
    // Minimum gap to show separate values

    // Draw the horizontal slider track across the screen
    u8g2.drawLine(SLIDER_START_X, SLIDER_Y, SLIDER_END_X, SLIDER_Y);

    // Draw small vertical markers at the ends of the slider track
    u8g2.drawLine(SLIDER_START_X, SLIDER_Y - END_MARKER_HEIGHT, SLIDER_START_X,
                  SLIDER_Y + END_MARKER_HEIGHT);
    u8g2.drawLine(SLIDER_END_X, SLIDER_Y - END_MARKER_HEIGHT, SLIDER_END_X,
                  SLIDER_Y + END_MARKER_HEIGHT);

    // Calculate x-positions for the min and max indicators
    int minIndicatorX = map(smallValue, min, max, SLIDER_START_X, SLIDER_END_X);
    int maxIndicatorX = map(bigValue, min, max, SLIDER_START_X, SLIDER_END_X);

    // Draw vertical indicators for the current min and max values
    drawIndicator(minIndicatorX, SLIDER_Y, INDICATOR_HEIGHT);
    drawIndicator(maxIndicatorX, SLIDER_Y, INDICATOR_HEIGHT);

    // Prepare strings to display the slider values
    char minStr[12];
    char maxStr[12];

    // Decide how to display values based on their difference
    if ((bigValue - smallValue) > VALUE_GAP_THRESHOLD) {
      // If values are far apart, show them separately above each indicator
      sprintf(minStr, "%d", smallValue);
      sprintf(maxStr, "%d", bigValue);
      u8g2.drawStr(minIndicatorX - 4, SLIDER_Y - 5, minStr);
      u8g2.drawStr(maxIndicatorX - 4, SLIDER_Y - 5, maxStr);
    } else {
      // If values are close, show a combined "min - max" string centered above
      sprintf(minStr, "%d - %d", smallValue, bigValue);
      int textX = Clamp(minIndicatorX - 4, SLIDER_START_X, SLIDER_END_X - 15);
      u8g2.drawStr(textX, SLIDER_Y - 10, minStr);
    }
  }

  /**
   * @brief Handle user input for the slider menu
   * @param input Input value from the user (UP, DOWN, SELECT)
   * @return NO_INPUT if no action is needed, otherwise returns a status code
   */
  uint8_t HandleInput(uint8_t input) override {
    if (input == UP) {
      // Increment value based on current setting unit
      if (current_setting_unit == 0 && smallValue < max - 1) {
        smallValue++;
      } else if (current_setting_unit == 1 && bigValue < max) {
        bigValue++;
      }
    } else if (input == DOWN) {
      // Decrement value based on current setting unit
      if (current_setting_unit == 0 && smallValue > min) {
        smallValue--;
      } else if (current_setting_unit == 1 && bigValue > min + 1) {
        bigValue--;
      }
    } else if (input == SELECT) {
      current_setting_unit++;
    }

    // Ensure they stay in range/in order with each other
    if ((smallValue >= bigValue) && current_setting_unit == 1) {
      smallValue = bigValue - 1;
      if ((smallValue >= bigValue) && current_setting_unit == 0) {
        bigValue = smallValue + 1;
      }
    }
    if (bigValue <= smallValue) {
      bigValue = smallValue + 1;
    }

    return NO_INPUT;
  }

private:
  const int SLIDER_Y = SCREEN_HEIGHT - 20;    // Y-position of the slider track
  const int SLIDER_START_X = 10;              // Left edge of the slider
  const int SLIDER_END_X = SCREEN_WIDTH - 10; // Right edge of the slider
  const int END_MARKER_HEIGHT = 2; // Height of end markers above/below track
  const int INDICATOR_HEIGHT = 3;  // Height of indicators above/below track
  const int VALUE_GAP_THRESHOLD = 11;

  uint8_t current_setting_unit = 0;
  uint8_t screen_id; // Unique identifier for the screen
  NavInfo *nav_info; // Pointer to navigation info

  int smallValue; // Small slider value (0-100 range)
  int bigValue;   // Big slider value (0-100 range)

  const int min = 0;
  const int max = 100;

  // Helper function to draw a vertical indicator at a given position
  void drawIndicator(int x, int y, int height) {
    u8g2.drawLine(x, y - height, x, y + height);
  }
};

/**
 * @class SettingsList
 * @brief Class representing a list of settings menu items
 * @details This class provides functionality to navigate through a list of menu
 * items and select individual items.
 */
class SettingsList : public Screen {
public:
  /**
   * @brief Constructor for SettingsList
   * @param screen_id Unique identifier for the screen
   * @param num_items Number of items in the menu
   * @param items Pointer to array of MenuItem objects
   * @param nav_info Navigation information pointer
   * @param new_current_item Initial selected item index (default: 0)
   */
  SettingsList(uint8_t screen_id, uint8_t num_items, MenuItem *items,
               NavInfo *nav_info,
               uint8_t new_current_item = 0)
      : Screen(screen_id),                  // Initialize the base class
        items(items), num_items(num_items), // Use the provided number of items
        nav_info(nav_info),                 // Initialize the NavInfo
        current_item(new_current_item) {}

  /**
   * @brief Handle user input for the settings list
   * @param input Input value from the user (UP, DOWN, SELECT)
   * @return ID of selected item if SELECT is pressed, 0 otherwise
   */
  uint8_t HandleInput(uint8_t input) override {
    switch (input) {
    case UP:
      current_item = Wrap(current_item + 1, 0, num_items - 1);
      break;
    case DOWN:
      current_item = Wrap(current_item - 1, 0, num_items - 1);
      break;
    case SELECT:
      return items[current_item].GetId();
      break;
    }
    return 0;
  }

  /**
   * @brief Draw the settings list on the display
   * @details This method renders the current selected menu item and its
   * neighbor. It draws icons and titles for the current and next items in the
   * list.
   */
  void Draw() override {
    if (num_items < 2) {
      return;
    }

    u8g2.setFontMode(1);
    u8g2.setFont(u8g_font_7x13B);
    u8g2.drawXBMP(5, 16, 14, 14, items[current_item].GetIcon());
    u8g2.drawStr(25, 28, items[current_item].GetTitle());

    if (current_item + 1 < num_items) {
      u8g2.setFont(u8g_font_5x8);
      u8g2.drawXBMP(5, 33, 14, 14, items[current_item + 1].GetIcon());
      u8g2.drawStr(25, 44, items[current_item + 1].GetTitle());
    }
  }

private:
  MenuItem *items;      /**< Pointer to the array of menu items */
  NavInfo *nav_info;    /**< Navigation information object */
  size_t num_items;     /**< Number of items in the menu */
  uint8_t current_item; /**< Index of the currently selected item */
};

/**
 * @class BaseUi
 * @brief Base class for UI elements with title, description, and time
 * @details This class provides a foundation for creating UI components that
 * include title text, description text, and time information.
 */
class BaseUi {
public:
  /**
   * @brief Constructor for BaseUi
   * @param t Title string
   * @param d Description string
   * @param c InternalTime object
   */
  BaseUi(char *t, char *d, InternalTime *c) : title(t), desc(d), time(c) {}

  /**
   * @brief Update the title and description
   * @param new_title New title string
   * @param new_desc New description string
   */
  void update(char *new_title, char *new_desc) {
    title = new_title;
    desc = new_desc;
  }

  /**
   * @brief Draw the base UI elements on the display
   * @details This method renders the title, time information, and description
   * on the screen. It also draws a border around the UI area.
   */
  void Draw() {
    u8g2.setFontMode(1);
    u8g2.drawBox(0, 0, SCREEN_WIDTH, 10);
    u8g2.setDrawColor(2);
    u8g2.setFont(u8g_font_baby);
    u8g2.drawStr(0, 7, title);

    // Draw the time if available
    if (time != NULL) {
      char time_str[20];
      sprintf(time_str, "%02d:%02d", time->GetHour(), time->GetMinute());
      u8g2.drawStr(103, 7, time_str);
    }

    u8g2.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10);
    u8g2.drawStr(0, SCREEN_HEIGHT - 2, desc);
  }

private:
  char *title;        /**< Pointer to the title string */
  char *desc;         /**< Pointer to the description string */
  InternalTime *time; /**< Pointer to the internal time object */
};
#endif // SCREENS_H
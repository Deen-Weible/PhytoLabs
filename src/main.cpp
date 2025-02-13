#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

// Screen setup
U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/18, /* data=*/23, /* cs=*/15, /* dc=*/4, /* reset=*/2);
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define CLOCK_SPEED 400000

// Buttons
#define BUTTON_DOWN_PIN 25
#define BUTTON_SELECT_PIN 32
#define BUTTON_UP_PIN 33

int button_down_clicked = 0;
int button_select_clicked = 0;
int button_up_clicked = 0;

// Menu info
int item_selected = 0;                          // which item in the menu is selected
int item_sel_previous;                          // previous item - used in the menu screen to draw the item before the selected one
int item_sel_next;                              // next item - used in the menu screen to draw next item after the selected one
int current_screen = 0;                         // 0 = menu, 1 = screenshot, 2 = qr
const int NUM_ITEMS = 8;                        // number of items in the list and also the number of screenshots and screenshots with QR codes (other screens)
const int MAX_ITEM_LENGTH = 20;                 // maximum characters for the item name
char menu_items[NUM_ITEMS][MAX_ITEM_LENGTH] = { // array with item names
    {"3D Cube"},
    {"Battery"},
    {"Dashboard"},
    {"Fireworks"},
    {"GPS Speed"},
    {"Big Knob"},
    {"Park Sensor"},
    {"Turbo Gauge"}};

void drawUI(char *title, char *description)
{
  // Drawing "Simple UI" text
  u8g2.setFont(u8g_font_baby);
  // u8g2.drawStr(0, 15, title);

  // Activating transparent font mode and drawing a line
  u8g2.setFontMode(1);
  u8g2.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10);
  // u8g2.drawBox(0, SCREEN_HEIGHT -10, SCREEN_WIDTH, SCREEN_HEIGHT);
  u8g2.setDrawColor(2);
  u8g2.drawStr(0, SCREEN_HEIGHT - 2, description);
}

/* draw something on the display with the `firstPage()`/`nextPage()` loop*/
void updateDisplay()
{
  u8g2.firstPage();
  do
  {
    drawUI("Simple UI", "This is a simple user interface.");

    if (current_screen == 0)
    { // MENU SCREEN

      // up and down buttons only work for the menu screen
      if ((digitalRead(BUTTON_UP_PIN) == LOW) && (button_up_clicked == 0))
      {                                    // up button clicked - jump to previous menu item
        item_selected = item_selected - 1; // select previous item
        button_up_clicked = 1;             // set button to clicked to only perform the action once
        if (item_selected < 0)
        { // if first item was selected, jump to last item
          item_selected = NUM_ITEMS - 1;
        }
      }
      else if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && (button_down_clicked == 0))
      {                                    // down button clicked - jump to next menu item
        item_selected = item_selected + 1; // select next item
        button_down_clicked = 1;           // set button to clicked to only perform the action once
        if (item_selected >= NUM_ITEMS)
        { // last item was selected, jump to first menu item
          item_selected = 0;
        }
      }

      if ((digitalRead(BUTTON_UP_PIN) == HIGH) && (button_up_clicked == 1))
      { // unclick
        button_up_clicked = 0;
      }
      if ((digitalRead(BUTTON_DOWN_PIN) == HIGH) && (button_down_clicked == 1))
      { // unclick
        button_down_clicked = 0;
      }

      item_sel_previous = item_selected - 1;
      if (item_sel_previous < 0)
      {
        item_sel_previous = NUM_ITEMS - 1;
      } // previous item would be below first = make it the last
      item_sel_next = item_selected + 1;
      if (item_sel_next >= NUM_ITEMS)
      {
        item_sel_next = 0;
      } // next item would be after last = make it the first
    }
    u8g2.drawStr(60, 30, menu_items[item_sel_previous]);
  } while (u8g2.nextPage());
}

void loop(void)
{
  updateDisplay();
}

/* u8g2.begin() is required and will sent the setup/init sequence to the display */
void setup(void)
{
  u8g2.begin();
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  SPI.setClockDivider(400000); // SPI clock divider
  u8g2.setBusClock(400000);    // SPI clock divider
}

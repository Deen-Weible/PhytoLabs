#include <Arduino.h>
#include <U8g2lib.h>
#include <Helpers.h>

class BaseUi {
public:
  char* title;
  char* desc;
	InternalTime time;

	void init(char* t, char* d, InternalTime &c) {
		title = t;
		desc = d;
		time = c;
	}

  void draw() {
    u8g2.setFontMode(1);
    u8g2.drawBox(0, 0, SCREEN_WIDTH, 10);
    u8g2.setDrawColor(2);
    u8g2.setFont(u8g_font_baby);
    u8g2.drawStr(0, 7, title);

    char time_str[20];
    sprintf(time_str, "%02d:%02d", time.current_hour, time.current_minute);
    u8g2.drawStr(103, 7, time_str);

    u8g2.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10);
    u8g2.drawStr(0, SCREEN_HEIGHT - 2, desc);
  };
};
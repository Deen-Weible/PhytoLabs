# PhytoLabs
## An automated farming system for ESP32 — Made with grant funding and love :3
## Built with support from my local school
## ⚠️ Important Note: This project is currently unfinished and requires manual intervention to compile

### How to get it working:
1. Create a new file named `WiFiInfo.h` in the `/include/` directory.
2. Add the following content (replace placeholders with your actual Wi-Fi details):
```cpp
#ifndef WIFIINFO_H
#define WIFIINFO_H

const char WIFI_SSID[] PROGMEM = "YourSSIDHere";
const char WIFI_PASSWORD[] PROGMEM = "YourPasswordHere";

#endif // WIFIINFO_H
```

> 💡 *Note: This file is intentionally excluded from version control (via `.gitignore`) so I don't need to use a dev network. Always update it with your own network details before compiling.*

## 📚 Project Overview
PhytoLabs is an open-source automated farming system built for ESP32 microcontrollers. It's designed for easy, flexible control and configuration of sensors and relays.

## License
This project is licensed under the GNU General Public License v3.0 (GPLv3).
See [LICENSE](LICENSE) for full terms.

(Always refer to the gpl site as it cannot be f'd up by me)

👉 Feel free to modify, share, or extend this project — just give credit and preserve open-source principles!
#ifndef WIFISETTINGS 
#define WIFISETTINGS

#include <WiFi.h>
#include <DNSServer.h>
// #include "Helpers.h"

// Assuming these are defined elsewhere (e.g., WiFiInfo.h or main.cpp)
// extern IPAddress subnetMask;
// extern const int WIFI_CHANNEL;
// extern const int MAX_CLIENTS;
//
void StartWiFi(const wifi_mode_t mode, const char *ssid, const char *password,
               const IPAddress &localIP = IPAddress(0, 0, 0, 0), 
               const IPAddress &gatewayIP = IPAddress(0, 0, 0, 0));

void SetupDNS(DNSServer &dnsServer, const IPAddress &localIP);

#endif // WIFISETTINGS 

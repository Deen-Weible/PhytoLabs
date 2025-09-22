#ifndef WIFISETTINGS 
#define WIFISETTINGS

#include <WiFi.h>
#include <DNSServer.h>

bool StartWiFi(const bool initialMode, const char *ssid, const char *password,
               const IPAddress &localIP = IPAddress(1, 2, 3, 4), 
               const IPAddress &gatewayIP = IPAddress(1, 2, 3, 4));

void SetupDNS(DNSServer &dnsServer, const IPAddress &localIP);

void SetupCaptivePortal(DNSServer &dnsServer, const IPAddress &localIP);


#endif // WIFISETTINGS 

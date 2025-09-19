#include "WifiSettings.h"

bool StartWiFi(const wifi_mode_t mode, const char *ssid, const char *password,
               const IPAddress &localIP, const IPAddress &gatewayIP) {
  WiFi.mode(mode);
  if (mode == WIFI_AP) {
    WiFi.softAPConfig(localIP, gatewayIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssid, password, 6, 0, 4);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    return true;
  } else if (mode == WIFI_STA) {
    WiFi.begin(ssid, password);
  }
  return false;
}

void SetupCaptivePortal(DNSServer &dnsServer, const IPAddress &localIP) {
  dnsServer.setTTL(3600);
  dnsServer.start(53, "*", localIP);
}

void SetupDNS(DNSServer &dnsServer, const IPAddress &localIP) {
  dnsServer.setTTL(3600);
  dnsServer.start(53, "*", WiFi.softAPIP());
}


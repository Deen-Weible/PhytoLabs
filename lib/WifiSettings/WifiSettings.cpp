#include "WifiSettings.h"

bool StartWiFi(const bool initialMode, const char *ssid, const char *password,
               const IPAddress &localIP, const IPAddress &gatewayIP) {
  Serial.println("Bingus");
  wifi_mode_t mode = (initialMode == 0) ? WIFI_AP : WIFI_STA;
  WiFi.mode(mode);

  // If ssid is blank, fallback to Configuration
  const char *effectiveSSID =
      (ssid == nullptr || strlen(ssid) == 0) ? "PhytoLabs Configuration" : ssid;
  const char *effectivePass =
      (ssid == nullptr || strlen(ssid) == 0) ? "" : password;

  if (mode) {
    WiFi.softAPConfig(localIP, gatewayIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(effectiveSSID, effectivePass, 6, 0, 4);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    return true;
  } else {
    WiFi.begin(effectiveSSID, effectivePass);
    unsigned long startAttemptTime = millis();

    // Wait for connection with a 10‑second timeout
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttemptTime < 10000) {
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
      return true;
    } else {
      // Fallback to AP mode if connection fails
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(localIP, gatewayIP, IPAddress(255, 255, 255, 0));
      WiFi.softAP(effectiveSSID, password, 6, 0, 4);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      return true;
    }
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

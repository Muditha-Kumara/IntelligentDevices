#include "WiFiManager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password)
    : _ssid(ssid), _password(password) {}

void WiFiManager::begin() {
    WiFi.begin(_ssid, _password);
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiManager::ensureConnection() {
    if (isConnected()) {
        return true;
    }
    WiFi.disconnect();
    WiFi.begin(_ssid, _password);
    unsigned long wifiStart = millis();
    int wifiTimeout = 10000;
    while (!isConnected() && millis() - wifiStart < wifiTimeout) {
        delay(500);
    }
    return isConnected();
}

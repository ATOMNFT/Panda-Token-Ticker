#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include <WiFiManager.h>  // Install via Library Manager
#include <TFT_eSPI.h>

extern TFT_eSPI tft;  // Provided by main sketch

void setupWiFi() {
  WiFiManager wm;
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Starting WiFi...", tft.width() / 2, 20);

  bool res;
  res = wm.autoConnect("PANDA-TICKER", "trackmypanda");  // AP name & Password for initial setup

  if (!res) {
    Serial.println("[!] Failed to connect. Restarting...");
    tft.drawString("WiFi Failed. Rebooting", tft.width() / 2, 50);
    delay(3000);
    ESP.restart();
  } else {
    Serial.println("[+] WiFi connected!");
    tft.drawString("WiFi connected!", tft.width() / 2, 50);
    delay(1000);
  }
}

#endif

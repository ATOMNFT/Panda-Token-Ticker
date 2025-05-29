// ####################
//      Created by                                                   
// ╔═╗╔╦╗╔═╗╔╦╗╔╗╔╔═╗╔╦╗  
// ╠═╣ ║ ║ ║║║║║║║╠╣  ║   
// ╩ ╩ ╩ ╚═╝╩ ╩╝╚╝╚   ╩   
// ####################   

/* FLASH SETTINGS
Board: LOLIN D32
Flash Frequency: 80MHz
Partition Scheme: Minimal SPIFFS
*/

#include <WiFi.h>
#include "wifi_setup.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include "panda_logo.h"

TFT_eSPI tft = TFT_eSPI();  // TFT instance

const char* apiUrl = "https://api.dexscreener.com/latest/dex/pairs/abstract/0xdc087d63bc59ae8692f6cbb0f2d8a1828a97c819";

// Helper function to format large numbers like 31.4K, 2.5M, etc.
String formatPrettyNumber(float num) {
  if (num >= 1e9) return String(num / 1e9, 1) + "B";
  if (num >= 1e6) return String(num / 1e6, 1) + "M";
  if (num >= 1e3) return String(num / 1e3, 1) + "K";
  return String(num, 2);
}

// Draw JPEG callback for TJpg_Decoder
bool drawJpgCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  tft.pushImage(x, y, w, h, bitmap);
  return true;
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(drawJpgCallback);

  setupWiFi();  // Auto AP-mode config and connection
  }

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(apiUrl);
    int httpCode = http.GET();

    if (httpCode == 200) {
      String payload = http.getString();
      Serial.println("---- RAW JSON ----");
      Serial.println(payload);

      DynamicJsonDocument doc(6144);
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.println("JSON parsing failed: " + String(error.c_str()));
        delay(10000);
        return;
      }

      JsonObject pair = doc["pairs"][0];

      const char* tokenName = pair["baseToken"]["name"] | "Unknown";
      const char* tokenSymbol = pair["baseToken"]["symbol"] | "???";
      const char* priceUsd = pair["priceUsd"] | "0.00";
      float priceChange = pair["priceChange"]["h24"] | 0.0;
      String priceChangeStr = String(priceChange, 2);
      float liquidityRaw = pair["liquidity"]["usd"] | 0.0;
      String liquidityPretty = formatPrettyNumber(liquidityRaw);

      Serial.printf("Token: %s (%s)\n", tokenName, tokenSymbol);
      Serial.printf("Price: $%s\n", priceUsd);
      Serial.printf("24h Change: %s%%\n", priceChangeStr.c_str());
      Serial.printf("Liquidity: $%s\n", liquidityPretty.c_str());

      tft.fillScreen(TFT_BLACK);

      // Title
      tft.setTextDatum(TC_DATUM);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("PANDA TICKER", tft.width() / 2, 5);

      // Draw panda image (centered below title)
      TJpgDec.drawJpg((tft.width() - 38) / 2, 25, panda_logo_38_jpg, panda_logo_38_jpg_len);  // Panda Image

      // Reset text settings and print data below image
      tft.setTextDatum(TL_DATUM);
      tft.setTextSize(2);

      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.drawString(String(tokenName), 20, 100);

      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("USD Price:", 20, 130);

      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextSize(3);
      tft.drawString("$" + String(priceUsd), 20, 150);
      tft.setTextSize(2);

      if (priceChange >= 0)
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
      else
        tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("24h Change: " + priceChangeStr + "%", 20, 190);

      tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
      tft.drawString("Liquidity: $" + liquidityPretty, 20, 220);

    } else {
      Serial.printf("HTTP error code: %d\n", httpCode);
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("HTTP Error", 20, 60);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("WiFi Disconnected", 20, 60);
  }

  delay(30000);  // Refresh every 30 seconds
}

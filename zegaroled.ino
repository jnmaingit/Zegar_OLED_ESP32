#include <WiFi.h>
#include <WiFiUdp.h>
#include <time.h>
#include <ArduinoOTA.h>
#include <U8g2lib.h>

// OLED 0.42" – 128x64 bufora, widoczna część 72x40 px u góry
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);

// Wi-Fi dane
const char* ssid = "ASUS";
const char* password = "2525252525";

// NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 2 * 3600;  // UTC+2
const int daylightOffset_sec = 0;

// Ekran: widoczne 72x40 pikseli, offsetY=0 (górna część bufora)
const int viewWidth = 72;
const int viewHeight = 40;
const int offsetX = (128 - viewWidth) / 2;  // 28
const int offsetY = 0;                      // pokazujemy górną część bufora
const int centerX = offsetX + viewWidth / 2; // 64
const int centerY = offsetY + 43;             // przesunięcie środka zegara 47 px w dół
const int radius = 19;                         // powiększony promień tarczy (z 16 na 18)

void setupWiFiAndTime() {
  WiFi.begin(ssid, password);
  Serial.print("Laczenie z WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nPolaczono z siecia WiFi!");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Blad pobierania czasu!");
  } else {
    Serial.println("Czas z NTP zsynchronizowany.");
  }
}

void setupOTA() {
  ArduinoOTA.setHostname("ESP32C3-OLED");
  ArduinoOTA.setPassword("jacek2");

  ArduinoOTA.onStart([]() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(offsetX + 5, offsetY + 20, "OTA Start...");
    u8g2.sendBuffer();
  });

  ArduinoOTA.onEnd([]() {
    u8g2.clearBuffer();
    u8g2.drawStr(offsetX + 5, offsetY + 20, "OTA Done!");
    u8g2.sendBuffer();
  });

  ArduinoOTA.begin();
  Serial.println("OTA gotowe.");
}

void drawHand(float angleDeg, int length, uint8_t thickness = 1) {
  float angleRad = (angleDeg - 90) * PI / 180;
  int x = centerX + cos(angleRad) * length;
  int y = centerY + sin(angleRad) * length;
  for (int i = 0; i < thickness; i++) {
    u8g2.drawLine(centerX + i, centerY + i, x + i, y + i);
  }
}

void drawAnalogClock(struct tm* timeinfo) {
  int sec = timeinfo->tm_sec;
  int min = timeinfo->tm_min;
  int hr = timeinfo->tm_hour % 12;

  float secAngle = sec * 6;
  float minAngle = min * 6 + sec * 0.1;
  float hrAngle = hr * 30 + min * 0.5;

  // Rysujemy obwódkę tarczy
  u8g2.drawCircle(centerX, centerY, radius, U8G2_DRAW_ALL);

  // Rysujemy wskazówki
  drawHand(hrAngle, radius - 7, 2);
  drawHand(minAngle, radius - 3, 1);
  drawHand(secAngle, radius - 1, 1);
}

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setContrast(255);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(offsetX + 5, offsetY + 20, "Laczenie...");
  u8g2.sendBuffer();

  setupWiFiAndTime();
  setupOTA();
}

void loop() {
  ArduinoOTA.handle();

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    u8g2.clearBuffer();
    drawAnalogClock(&timeinfo);
    u8g2.sendBuffer();
  }

  delay(1000);
}

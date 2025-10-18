#include <WiFi.h>
#include <WiFiManager.h> 
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include "Ubidots.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "secrets.h"  

// === KONFIGURASI ===
#define DEVICE_LABEL "ESP32_GPS"

#define RXD2 16
#define TXD2 17
#define SD_CS 5
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Ubidots ubidots(UBIDOTS_TOKEN, UBI_TCP);  // Token diambil dari secrets.h
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
RTC_DS3231 rtc;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

float totalJarak = 0.0;
float prevLat = 0.0, prevLon = 0.0;
bool firstFix = true;

// === FUNGSI HITUNG JARAK (KM) ===
float hitungJarak(float lat1, float lon1, float lat2, float lon2) {
  const float R = 6371.0; // radius bumi (km)
  float dLat = radians(lat2 - lat1);
  float dLon = radians(lon2 - lon1);
  float a = sin(dLat / 2) * sin(dLat / 2) +
            cos(radians(lat1)) * cos(radians(lat2)) *
            sin(dLon / 2) * sin(dLon / 2);
  float c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return R * c;
}

// === SETUP ===
void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // === OLED ===
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED tidak terdeteksi!");
  } else {
    display.clearDisplay();
    display.setTextSize(1, 2);
    display.setTextColor(SSD1306_WHITE);

    String teks = "Slow Hustle";
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(teks, 0, 0, &x1, &y1, &w, &h);
    int xPos = (128 - w) / 2;
    int yPos = (64 - h) / 2;

    display.setCursor(xPos, yPos);
    display.println(teks);
    display.display();

    delay(3000);
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("GPS Tracker Init...");
    display.display();
  }

  // === SD CARD ===
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card gagal diinisialisasi!");
  } else {
    Serial.println("SD Card DONE.");
  }

  // === RTC ===
  if (!rtc.begin()) {
    Serial.println("RTC tidak terdeteksi!");
  } else {
    rtc.adjust(DateTime(__DATE__, __TIME__));
    Serial.println("RTC DONE.");
  }

  // === WIFI MANAGER ===
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);
  if (!wifiManager.autoConnect("ESP32_Config", "12345678")) {
    Serial.println("Gagal konek WiFi, reset ESP!");
    ESP.restart();
  }

  Serial.println("\nWiFi tersambung!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi OK!");
  display.println(WiFi.localIP());
  display.display();
  delay(1000);
}

// === LOOP ===
void loop() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isUpdated()) {
    float lat = gps.location.lat();
    float lon = gps.location.lng();
    float spd = gps.speed.kmph();
    float dir = gps.course.deg();
    DateTime now = rtc.now();

    // === HITUNG JARAK ===
    if (!firstFix) {
      float jarak = hitungJarak(prevLat, prevLon, lat, lon);
      totalJarak += jarak;
    } else {
      firstFix = false;
    }
    prevLat = lat;
    prevLon = lon;

    // === SERIAL MONITOR ===
    Serial.println("==========");
    Serial.printf("Lat: %.6f, Lon: %.6f\n", lat, lon);
    Serial.printf("Speed: %.2f km/h | Arah: %.2f°\n", spd, dir);
    Serial.printf("Total Jarak: %.3f km\n", totalJarak);
    Serial.print("Waktu: ");
    Serial.println(now.timestamp());

    // === OLED ===
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Lat: "); display.println(lat, 6);
    display.print("Lon: "); display.println(lon, 6);
    display.print("Spd: "); display.print(spd, 1); display.println(" km/h");
    display.print("Jrk: "); display.print(totalJarak, 2); display.println(" km");
    display.display();

    // === SIMPAN SD ===
    File file = SD.open("/gps_log.txt", FILE_APPEND);
    if (file) {
      file.printf("%02d/%02d/%04d,%02d:%02d:%02d,%.6f,%.6f,%.2f,%.2f,%.3f\n",
                  now.day(), now.month(), now.year(),
                  now.hour(), now.minute(), now.second(),
                  lat, lon, spd, dir, totalJarak);
      file.close();
      Serial.println("Data disimpan ke SD");
    } else {
      Serial.println("Gagal menulis ke SD Card");
    }

    // === KIRIM UBIDOTS ===
    char context[100];
    sprintf(context, "lat=%.6f$lng=%.6f", lat, lon);

    unsigned long waktuEpoch = gps.time.value();
    ubidots.add("position", waktuEpoch, context);
    ubidots.add("speed", spd);
    ubidots.add("direction", dir);
    ubidots.add("distance", totalJarak);

    bool sent = ubidots.send(DEVICE_LABEL);

    if (sent) {
      Serial.println("Data terkirim ke Ubidots!");
    } else {
      Serial.println("Gagal kirim data ke Ubidots.");
    }

    delay(5000); // kirim tiap 5 detik
  }
}

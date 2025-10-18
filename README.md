# Pengukuran Kecepatan, Titik Koordinat, Heading, dan Jarak pada Kapal RC Boat (ESP32)

Memantau kecepatan, arah (heading), titik koordinat, dan jarak tempuh kapal RC Boat menggunakan
ESP32 + GPS NEO-6M + OLED SSD1306 + RTC DS3231 + microSD + Ubidots.
Sistem menampilkan data lokasi dan kecepatan secara realtime di OLED, menyimpan log ke SD Card, serta mengirim data ke Ubidots untuk pemantauan online berbasis IoT.
---

## Fitur
- Menampilkan data GPS secara realtime di OLED bertahap:
    i. LAT dan LON → posisi koordinat kapal  
    ii. SPD → kecepatan kapal (km/h)  
    iii. DIR → arah gerak kapal (°)  
    iv. JARAK → total jarak tempuh (km)
- GPS NEO-6M: update data posisi dan kecepatan setiap 1 detik.
- RTC DS3231: mencatat waktu akurat untuk setiap data log.
- microSD: simpan data ke gps_log.txt tiap interval tertentu.
- Ubidots: kirim data GPS setiap 5 detik (jika koneksi tersedia).
- WiFiManager: portal konfigurasi WiFi otomatis saat belum terhubung.
- Haversine: hitung jarak antar titik koordinat kapal.

---

## Hardware
- **ESP32 Devkit V1**  
- **GPS NEO-6M** — TX ke `GPIO16`, RX ke `GPIO17`
- **OLED SSD1306 I²C (0x3C)** — SDA ke `GPIO21`, SCL ke `GPIO22`
- **RTC DS3231 (I²C)**
- **SD Card Module** — CS ke `GPIO5`
- **Sumber daya:** Baterai Li-Ion atau Powerbank

---

## Dashboard Ubidots
Menampilkan Peta pergerakan kapal, Grafik kecepatan & arah secara real-time, Rekaman data jarak tempuh
  
## Library
`WiFi`,`WiFiManager`,`TinyGPSPlus`,`HardwareSerial`,`Ubidots`,`Wire`,`SPI`,`SD`,`RTClib`,`Adafruit_GFX`,`Adafruit_SSD1306`.

## Build & Setup
1. Tambahkan file lokal **`secrets.h`** (jangan di-commit ke GitHub):
   ```cpp
   #pragma once
   #define UBIDOTS_TOKEN "ISI_TOKEN_UBIDOTS_KAMU"

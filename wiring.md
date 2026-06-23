# Skema Rangkaian — Smart Parking AI

## Daftar Pin ESP32

| Komponen | Pin Komponen | Pin ESP32 |
|---|---|---|
| HC-SR04 #1 (Masuk) | VCC | 5V |
| | GND | GND |
| | TRIG | GPIO 5 |
| | ECHO | GPIO 18 * |
| HC-SR04 #2 (Keluar) | VCC | 5V |
| | GND | GND |
| | TRIG | GPIO 17 |
| | ECHO | GPIO 16 |
| Servo (palang) | VCC (merah) | 5V (gunakan sumber eksternal jika servo "jitter") |
| | GND (coklat/hitam) | GND |
| | Signal (oranye) | GPIO 13 |
| OLED 128x64 (I2C) | VCC | 3.3V |
| | GND | GND |
| | SDA | GPIO 21 (default I2C ESP32) |
| | SCL | GPIO 22 (default I2C ESP32) |
| LED Hijau | Anoda (+) → resistor 220Ω → | GPIO 25 |
| | Katoda (-) | GND |
| LED Merah | Anoda (+) → resistor 220Ω → | GPIO 26 |
| | Katoda (-) | GND |

\* **Penting:** ECHO sensor HC-SR04 mengeluarkan sinyal 5V, sedangkan GPIO ESP32 hanya tahan 3.3V.
Gunakan **voltage divider** (resistor 1kΩ + 2kΩ) pada jalur ECHO setiap sensor untuk melindungi ESP32, atau gunakan modul HC-SR04 versi 3.3V-tolerant.

## Diagram Sambungan (skematik sederhana)

```
                         +5V ──┬───────────────┬──────────────┐
                               │               │              │
                         ┌─────┴─────┐   ┌─────┴─────┐   ┌─────┴─────┐
                         │ HC-SR04 #1 │   │ HC-SR04 #2 │   │   Servo   │
                         │  (MASUK)   │   │  (KELUAR)  │   │ (palang)  │
                         └─┬───┬─────┘   └─┬───┬─────┘   └─────┬─────┘
                           │   │            │   │               │
                     TRIG──┘   └──ECHO TRIG─┘   └──ECHO     Signal
                      │ (GPIO5)  (GPIO18*)│ (GPIO17) (GPIO16*) (GPIO13)
                      │                   │
                     GND                 GND

   ESP32 3.3V ── SDA(GPIO21) ── SCL(GPIO22) ── OLED 128x64 (I2C, addr 0x3C)

   ESP32 GPIO25 ── 220Ω ── LED Hijau (Anoda) ── (Katoda) ── GND
   ESP32 GPIO26 ── 220Ω ── LED Merah (Anoda) ── (Katoda) ── GND
```

> *GPIO18 dan GPIO16 pada jalur ECHO wajib melewati voltage divider sebelum masuk ke ESP32.

## Tata Letak Fisik yang Disarankan

- Sensor HC-SR04 #1 diarahkan ke jalur masuk, ditempatkan ~5-10 cm di atas permukaan agar mendeteksi mobil melintas (bukan manusia/objek kecil).
- Sensor HC-SR04 #2 di jalur keluar, dengan orientasi sama.
- Servo dipasang pada mekanisme palang (lengan kayu/plastik ringan).
- OLED dan kedua LED ditempatkan di panel indikator yang mudah dilihat penjaga/pengemudi.

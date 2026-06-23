# Smart Parking AI — ESP32 Edge AI

Proyek Akhir mata kuliah **Algoritma dan Pemrograman** — Tema #10: *Smart Parking (deteksi & prediksi slot)*.

Sistem parkir cerdas berbasis ESP32 yang mendeteksi kendaraan masuk/keluar secara otomatis, menggerakkan palang servo, menampilkan status di OLED, dan mengklasifikasikan tingkat kepadatan parkir (**SEPI / SEDANG / PADAT**) menggunakan algoritma **k-Nearest Neighbors (k-NN)** yang ditulis dari nol.

## Komponen (Pilar 3 — Hardware)

| Komponen | Jumlah | Fungsi |
|---|---|---|
| ESP32 DevKit | 1 | Mikrokontroler utama |
| Sensor Ultrasonik HC-SR04 | 2 | Deteksi kendaraan di pintu masuk & keluar |
| Motor Servo | 1 | Palang/gate otomatis |
| OLED 128x64 (I2C, SSD1306) | 1 | Tampilan status & prediksi AI |
| LED Hijau | 1 | Indikator slot tersedia |
| LED Merah | 1 | Indikator parkir penuh |
| Resistor 220Ω | 2 | Pembatas arus untuk masing-masing LED |

## Algoritma yang Diimplementasikan dari Nol (Pilar 1)

| Algoritma | Fungsi dalam Sistem | Kompleksitas |
|---|---|---|
| Circular buffer | Menyimpan riwayat jumlah mobil per slot waktu | O(1) per update |
| Moving average | Ekstraksi fitur tren okupansi parkir | O(W), W = ukuran jendela (konstan) |
| Euclidean distance | Jarak antar fitur untuk k-NN | O(n), n = jumlah sampel latih |
| Partial selection sort | Mengambil k tetangga terdekat | O(n·k) |
| Z-score | Deteksi anomali pembacaan sensor ultrasonik | O(1) per pembacaan |

## Komponen AI/ML (Pilar 2)

**k-Nearest Neighbors (k-NN)**, k = 3, dengan 2 fitur:
1. Okupansi saat ini (jumlah mobil / kapasitas)
2. Tren rata-rata bergerak okupansi (moving average)

Output klasifikasi: `SEPI`, `SEDANG`, `PADAT`.

## Struktur Repositori

```
smart-parking-ai/
├── src/
│   └── smart_parking_ai.ino      # Kode utama ESP32 (Arduino IDE / PlatformIO)
├── docs/
│   ├── wiring.md                 # Skema rangkaian & daftar pin
│   ├── analisis_kompleksitas.md  # Analisis Big-O & memori
│   └── laporan_template.md       # Kerangka laporan teknis (8-15 halaman)
├── data/
│   └── kalibrasi_dataset.md      # Cara merekam & melatih ulang dataset k-NN
├── README.md
└── LICENSE
```

## Cara Menjalankan

1. Install **Arduino IDE** (atau PlatformIO) + board package **ESP32**.
2. Install library dari Library Manager:
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
   - `ESP32Servo`
3. Hubungkan komponen sesuai [docs/wiring.md](docs/wiring.md).
4. Buka `src/smart_parking_ai.ino`, pilih board ESP32 yang sesuai, lalu upload.
5. Buka Serial Monitor (115200 baud) untuk melihat log event masuk/keluar dan prediksi AI.

## Evaluasi Akurasi

Sistem mencatat `totalPrediksi` dan `prediksiBenar` secara otomatis setiap 5 menit (dibandingkan label aktual dari jumlah mobil riil), lalu menampilkan persentase akurasi di OLED. Gunakan data ini untuk bagian evaluasi pada laporan teknis.

## Penanganan Kasus Tepi

- Sensor timeout/error (`pulseIn` gagal) → dibaca sebagai `999`, otomatis diabaikan.
- Lonjakan jarak akibat noise/objek lewat sekilas → disaring dengan deteksi anomali z-score.
- Double-counting saat kendaraan berhenti tepat di sensor → cooldown 3 detik + jeda tunggu sampai jarak kembali normal.
- Parkir penuh → palang tidak dibuka untuk kendaraan masuk baru, LED merah menyala.

## Lisensi

MIT — lihat [LICENSE](LICENSE).

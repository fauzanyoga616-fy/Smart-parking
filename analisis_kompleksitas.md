# Analisis Kompleksitas & Keterbatasan Memori

## Kompleksitas Waktu

| Fungsi | Kompleksitas | Keterangan |
|---|---|---|
| `bacaJarak()` | O(1) | Satu kali pulse trigger + pulseIn (dibatasi timeout 30ms) |
| `isAnomali()` | O(1) | Exponential moving average/variance, tidak menyimpan histori |
| `updateMovingAverage()` | O(W) = O(1) | W = `WINDOW_SIZE` = 3, konstan terlepas dari N |
| `euclideanDistance()` | O(f) = O(1) | f = `N_FEATURES` = 2, konstan |
| `knnClassify()` | O(n·k) | n = `MAX_SAMPLES` = 30 (hitung jarak ke semua sampel: O(n)), lalu partial selection sort untuk k=3 terkecil: O(n·k) |
| `loop()` per iterasi | O(n·k) | Didominasi oleh `knnClassify()`, dipanggil hanya saat ada event masuk/keluar — bukan setiap iterasi loop |

Karena n (jumlah sampel latih) dan k tetap kecil dan konstan (≤30 dan 3), seluruh sistem berjalan **near-O(1) secara praktis** dan jauh di bawah ambang real-time 1 detik yang disyaratkan tugas.

## Kompleksitas Ruang (Memori)

| Struktur Data | Ukuran | Total |
|---|---|---|
| `trainingData[MAX_SAMPLES]` | 30 × (2 float + 1 int) = 30 × 12 byte | ~360 byte |
| `historiBuffer[12][3]` | 36 × 4 byte (int) | ~144 byte |
| `rataRataSlot[12]` | 12 × 4 byte (float) | 48 byte |
| Variabel global lain | — | < 100 byte |

**Total RAM yang dipakai struktur AI/algoritma:** ± 700 byte — sangat kecil dibanding RAM ESP32 (~520 KB), menyisakan banyak ruang untuk stack WiFi/Bluetooth jika dikembangkan lebih lanjut.

## Keterbatasan ESP32 yang Relevan

- **ADC/timing**: `pulseIn()` bersifat blocking; pada sistem dengan banyak sensor sebaiknya gunakan interrupt-based timing agar tidak menghambat tugas lain.
- **Memori Flash**: dataset k-NN ditanam sebagai konstanta di flash (bukan RAM) sehingga tidak membebani RAM runtime.
- **Presisi waktu**: tanpa RTC/NTP, sistem menggunakan `millis()` untuk slot waktu relatif sejak boot — cukup untuk evaluasi tren jangka pendek, namun tidak merepresentasikan jam riil (lihat bagian "Pengembangan Lanjutan" di laporan jika ingin menambah modul NTP/RTC).

## Pilihan Struktur Data — Justifikasi

- **Circular buffer** dipilih untuk menyimpan riwayat per slot waktu karena operasi insert berjalan O(1) dan otomatis menimpa data terlama tanpa perlu shifting array (berbeda dengan array biasa yang butuh O(W) untuk shifting).
- **Array statis (bukan dynamic allocation)** dipakai untuk `trainingData` agar tidak ada fragmentasi heap pada mikrokontroler dengan RAM terbatas.
- **Partial selection sort** (bukan full sort) dipilih untuk k-NN karena hanya k elemen terkecil yang dibutuhkan — menghindari biaya sort penuh O(n log n) yang tidak perlu untuk n kecil.

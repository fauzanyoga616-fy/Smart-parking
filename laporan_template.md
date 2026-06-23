# Laporan Teknis — Smart Parking AI

> Template ini mengikuti Bagian H & I pada soal proyek akhir. Isi tiap bagian sesuai hasil implementasi dan pengujian kalian. Target panjang: 8–15 halaman.

## 1. Latar Belakang
Jelaskan masalah parkir yang ingin diselesaikan, kenapa Edge AI di ESP32 relevan dibanding solusi server.

## 2. Tujuan
- Mendeteksi kendaraan masuk/keluar secara otomatis.
- Mengklasifikasikan tingkat kepadatan parkir secara real-time menggunakan k-NN.
- Mengevaluasi akurasi prediksi terhadap kondisi aktual.

## 3. Diagram Alur Sistem
```
[ HC-SR04 Masuk/Keluar ] -> [ Filter Anomali (Z-score) ] -> [ Circular Buffer + Moving Average ]
        -> [ Ekstraksi Fitur ] -> [ k-NN Classifier ] -> [ OLED / LED / Servo ]
```
(Lampirkan diagram visual — boleh dari draw.io / Fritzing / tangan.)

## 4. Perangkat Keras
Lampirkan tabel komponen dan skema rangkaian — lihat `docs/wiring.md`.

## 5. Penjelasan Algoritma (Pilar 1)
Untuk setiap algoritma berikut, jelaskan **cara kerja**, **alasan pemilihan struktur data**, dan **potongan kode kunci**:
- Circular buffer
- Moving average
- Euclidean distance + partial selection sort

## 6. Komponen AI/ML (Pilar 2)
- Jelaskan fitur yang dipakai k-NN dan alasan pemilihannya.
- Jelaskan proses pengumpulan dataset latih (lihat `data/kalibrasi_dataset.md`).
- Jelaskan pemilihan nilai k=3 dan trade-off-nya.

## 7. Analisis Kompleksitas
Salin & elaborasi dari `docs/analisis_kompleksitas.md`. Sertakan pengukuran waktu eksekusi nyata (mis. `micros()` sebelum/sesudah `knnClassify()`).

## 8. Hasil Pengujian & Evaluasi Akurasi
- Tabel jumlah prediksi benar vs total prediksi (dari variabel `totalPrediksi` / `prediksiBenar`).
- Grafik distribusi label SEPI/SEDANG/PADAT terhadap waktu pengujian.
- Skenario edge case yang diuji (sensor dihalangi, objek kecil lewat, parkir penuh, dst.) dan hasilnya.

## 9. Kendala dan Solusi
Dokumentasikan masalah teknis yang ditemui (mis. noise sensor ultrasonik, servo jitter, kalibrasi threshold) dan cara mengatasinya.

## 10. Kesimpulan
Ringkas pencapaian terhadap capaian pembelajaran (Bagian B soal).

## 11. Lampiran
- Listing kode lengkap (atau tautan ke repositori).
- Dataset mentah hasil rekaman kalibrasi.
- Foto/video alat (tautan video demo 2–4 menit).

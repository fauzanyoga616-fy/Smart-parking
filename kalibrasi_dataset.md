# Kalibrasi & Dataset k-NN

Dataset latih (`trainingData[]` di `src/smart_parking_ai.ino`) saat ini berisi contoh awal dan **wajib diganti** dengan data hasil rekaman dari alat kalian sendiri (sesuai Bagian F soal: "Dataset & pelatihan — jelaskan asal data latih, direkam sendiri lewat ESP32").

## Cara Merekam Data

1. Tambahkan baris berikut di `loop()` (sementara, untuk mode kalibrasi):
   ```cpp
   Serial.print(jumlahMobil / (float)KAPASITAS_MAKS);
   Serial.print(",");
   Serial.println(rataRataSlot[slotWaktuAktif] / (float)KAPASITAS_MAKS);
   ```
2. Jalankan sistem selama beberapa jam/hari pada kondisi nyata (parkir sepi pagi, ramai siang, dst).
3. Salin output Serial Monitor ke file CSV, mis. `data/rekaman_mentah.csv`.
4. Untuk tiap baris data, beri label manual berdasarkan kondisi aktual saat itu:
   - `0` = SEPI (okupansi rendah)
   - `1` = SEDANG
   - `2` = PADAT
5. Pastikan dataset memiliki **minimal 2 kelas** (idealnya 3, sesuai jumlah label) dengan jumlah sampel yang seimbang.
6. Masukkan kembali data berlabel ke array `trainingData[]` dalam kode, lalu sesuaikan `numSamples`.

## Tips Validasi

- Pisahkan sebagian data sebagai data uji (mis. 80% latih, 20% uji) sebelum ditanam ke ESP32, untuk mengukur akurasi awal di laptop (bisa pakai Python/Excel) sebelum deployment.
- Setelah ditanam ke ESP32, akurasi real-time tetap dipantau otomatis lewat variabel `totalPrediksi` dan `prediksiBenar` (lihat README bagian "Evaluasi Akurasi").

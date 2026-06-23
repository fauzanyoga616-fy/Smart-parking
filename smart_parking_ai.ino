// ============================================================
//  SMART PARKING AI — ESP32 Edge AI
//  Proyek Akhir Algoritma & Pemrograman (Tema #10)
//
//  Komponen:
//   - ESP32 DevKit
//   - 2x Sensor Ultrasonik HC-SR04 (pintu masuk & keluar)
//   - 1x Servo (palang/gate)
//   - OLED 128x64 (I2C, SSD1306)
//   - 1x LED Hijau  + resistor 220ohm  -> indikator SLOT TERSEDIA
//   - 1x LED Merah  + resistor 220ohm  -> indikator PARKIR PENUH
//
//  Pilar yang diimplementasikan:
//   Pilar 1 (Algoritma dari nol):
//     - Circular buffer (penyangga data real-time)
//     - Moving average (ekstraksi fitur)
//     - Euclidean distance + sorting manual (untuk k-NN)
//     - Z-score (deteksi anomali pembacaan sensor)
//   Pilar 2 (AI/ML):
//     - k-Nearest Neighbors (k-NN) untuk klasifikasi kepadatan
//       parkir: SEPI / SEDANG / PADAT
//   Pilar 3 (Hardware):
//     - ESP32 + 2x HC-SR04 (input) + Servo, OLED, 2x LED (output)
// ============================================================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// ============================================================
//  OLED
// ============================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ============================================================
//  Pin Komponen
// ============================================================
#define TRIG_IN      5     // HC-SR04 pintu masuk
#define ECHO_IN      18
#define TRIG_OUT     17    // HC-SR04 pintu keluar
#define ECHO_OUT     16
#define SERVO_PIN    13    // Servo palang

#define LED_HIJAU    25    // Slot tersedia
#define LED_MERAH    26    // Parkir penuh
// Pasang masing-masing LED dengan resistor 220ohm secara seri ke GND

Servo gateServo;

// ============================================================
//  Parameter Sistem
// ============================================================
#define JARAK_TRIGGER_CM   3      // Jarak (cm) dianggap "ada mobil" di sensor
#define COOLDOWN_MS        3000   // Anti double-count
#define KAPASITAS_MAKS     4      // Jumlah slot parkir

int jumlahMobil = 0;
unsigned long lastEventMasuk  = 0;
unsigned long lastEventKeluar = 0;

// ============================================================
//  PILAR 1.A — CIRCULAR BUFFER (penyangga data real-time)
//  Menyimpan riwayat jumlah mobil per "slot waktu" (5 menit)
//  agar bisa dihitung rata-rata bergerak (moving average).
// ============================================================
#define TOTAL_SLOT_WAKTU   12     // 12 slot x 5 menit = representasi 1 jam berjalan
#define WINDOW_SIZE         3     // ukuran jendela moving average

int historiBuffer[TOTAL_SLOT_WAKTU][WINDOW_SIZE];
int indexBuffer[TOTAL_SLOT_WAKTU];
int countBuffer[TOTAL_SLOT_WAKTU];
float rataRataSlot[TOTAL_SLOT_WAKTU];

int slotWaktuAktif = 0;     // slot waktu berjalan (bertambah tiap 5 menit sejak boot)
unsigned long lastSlotUpdate = 0;
#define INTERVAL_SLOT_MS  300000UL   // 5 menit

void initBuffer() {
  for (int i = 0; i < TOTAL_SLOT_WAKTU; i++) {
    indexBuffer[i] = 0;
    countBuffer[i] = 0;
    rataRataSlot[i] = 0;
    for (int j = 0; j < WINDOW_SIZE; j++) historiBuffer[i][j] = 0;
  }
}

// Kompleksitas: O(WINDOW_SIZE) per update -> O(1) karena WINDOW_SIZE konstan
void updateMovingAverage(int slot, int nilai) {
  historiBuffer[slot][indexBuffer[slot]] = nilai;
  indexBuffer[slot] = (indexBuffer[slot] + 1) % WINDOW_SIZE;
  if (countBuffer[slot] < WINDOW_SIZE) countBuffer[slot]++;

  float total = 0;
  for (int i = 0; i < countBuffer[slot]; i++) total += historiBuffer[slot][i];
  rataRataSlot[slot] = total / countBuffer[slot];
}

// ============================================================
//  PILAR 1.B — EUCLIDEAN DISTANCE + SORTING (untuk k-NN)
// ============================================================
#define N_FEATURES   2
#define MAX_SAMPLES  30
#define K            3

struct Sample {
  float features[N_FEATURES];   // [0]=okupansi sekarang, [1]=tren rata-rata
  int   label;                  // 0=SEPI, 1=SEDANG, 2=PADAT
};

const char* labelNama[3] = {"SEPI", "SEDANG", "PADAT"};

// Dataset latih awal (hasil rekaman manual via Serial saat kalibrasi).
// Fitur dinormalisasi 0..1 : okupansi = jumlahMobil/KAPASITAS_MAKS
Sample trainingData[MAX_SAMPLES] = {
  {{0.00, 0.00}, 0}, {{0.00, 0.10}, 0}, {{0.25, 0.10}, 0},
  {{0.25, 0.25}, 1}, {{0.50, 0.25}, 1}, {{0.50, 0.40}, 1},
  {{0.75, 0.50}, 2}, {{0.75, 0.75}, 2}, {{1.00, 0.75}, 2},
  {{1.00, 1.00}, 2}, {{0.00, 0.00}, 0}, {{0.25, 0.00}, 0},
  {{0.25, 0.40}, 1}, {{0.50, 0.10}, 0}, {{0.50, 0.50}, 1},
  {{0.75, 0.25}, 1}, {{0.75, 1.00}, 2}, {{1.00, 0.50}, 2},
  {{0.00, 0.25}, 0}, {{0.25, 0.50}, 1}
};
int numSamples = 20;

// Kompleksitas: O(n) untuk menghitung semua jarak
float euclideanDistance(const float a[], const float b[], int n) {
  float sum = 0.0;
  for (int i = 0; i < n; i++) {
    float d = a[i] - b[i];
    sum += d * d;
  }
  return sqrt(sum);
}

void extractFeatures(int jumlah, float rata, float out[]) {
  out[0] = jumlah / (float)KAPASITAS_MAKS;
  out[1] = rata   / (float)KAPASITAS_MAKS;
}

// Kompleksitas: O(n*k) — selection sort parsial untuk ambil k jarak terkecil
int knnClassify(const float input[], int k) {
  float jarak[MAX_SAMPLES];
  int   idx[MAX_SAMPLES];

  for (int i = 0; i < numSamples; i++) {
    jarak[i] = euclideanDistance(input, trainingData[i].features, N_FEATURES);
    idx[i] = i;
  }

  int kE = (k < numSamples) ? k : numSamples;
  for (int i = 0; i < kE; i++) {
    int m = i;
    for (int j = i + 1; j < numSamples; j++) {
      if (jarak[j] < jarak[m]) m = j;
    }
    float tj = jarak[i]; jarak[i] = jarak[m]; jarak[m] = tj;
    int   ti = idx[i];   idx[i]   = idx[m];   idx[m]   = ti;
  }

  int vote[3] = {0, 0, 0};
  for (int i = 0; i < kE; i++) vote[trainingData[idx[i]].label]++;

  int best = 0;
  for (int i = 1; i < 3; i++) if (vote[i] > vote[best]) best = i;
  return best;
}

int labelAktif = 0;

void updatePrediksiAI() {
  float fitur[N_FEATURES];
  extractFeatures(jumlahMobil, rataRataSlot[slotWaktuAktif], fitur);
  labelAktif = knnClassify(fitur, K);
}

// ============================================================
//  PILAR 1.C — DETEKSI ANOMALI (Z-SCORE)
//  Menyaring pembacaan sensor ultrasonik yang melonjak/error
//  (mis. terhalang, noise, sensor lepas) agar tidak dihitung.
// ============================================================
#define ANOMALI_THRESHOLD 2.5
float rataJarakMasuk = 50.0, varJarakMasuk = 100.0;
float rataJarakKeluar = 50.0, varJarakKeluar = 100.0;

// Kompleksitas: O(1) per pembacaan (exponential moving stats)
bool isAnomali(long jarak, float &rata, float &varian) {
  if (jarak <= 0 || jarak == 999) return true;   // bacaan gagal/timeout
  float delta = jarak - rata;
  rata   += delta * 0.1;
  varian  = varian * 0.9 + delta * delta * 0.1;
  float stddev = sqrt(varian);
  if (stddev < 1.0) stddev = 1.0;
  float z = fabs(jarak - rata) / stddev;
  return (z > ANOMALI_THRESHOLD);
}

// ============================================================
//  Baca Sensor Ultrasonik
// ============================================================
long bacaJarak(int trig, int echo) {
  digitalWrite(trig, LOW);  delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long durasi = pulseIn(echo, HIGH, 30000); // timeout 30ms
  if (durasi == 0) return 999;              // dianggap "tidak terdeteksi"
  return durasi * 0.034 / 2;                // konversi ke cm
}

// ============================================================
//  Evaluasi Akurasi Model (opsional, untuk laporan)
// ============================================================
int totalPrediksi = 0, prediksiBenar = 0;

int labelDariJumlahMobilAktual(int j) {
  if (j == 0) return 0;
  if (j <= KAPASITAS_MAKS / 2) return 1;
  return 2;
}

void evaluasiPrediksi() {
  int riil = labelDariJumlahMobilAktual(jumlahMobil);
  totalPrediksi++;
  if (riil == labelAktif) prediksiBenar++;
}

// ============================================================
//  Aktuator: Servo, LED, OLED
// ============================================================
void bukaPalang() {
  gateServo.write(90);
  delay(2500);
  gateServo.write(0);
  delay(500);
}

void updateLED() {
  if (jumlahMobil < KAPASITAS_MAKS) {
    digitalWrite(LED_HIJAU, HIGH);
    digitalWrite(LED_MERAH, LOW);
  } else {
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_MERAH, HIGH);
  }
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("SMART PARKING AI");

  display.setCursor(0, 12);
  display.print("Mobil  : "); display.print(jumlahMobil);
  display.print(" / "); display.println(KAPASITAS_MAKS);

  display.setCursor(0, 24);
  display.print("Sisa   : "); display.println(KAPASITAS_MAKS - jumlahMobil);

  display.setCursor(0, 36);
  display.print("Status : "); display.println(labelNama[labelAktif]);

  display.setCursor(0, 48);
  if (jumlahMobil >= KAPASITAS_MAKS) {
    display.println("!! PARKIR PENUH !!");
  } else if (totalPrediksi > 0) {
    float akurasi = 100.0 * prediksiBenar / totalPrediksi;
    display.print("Akurasi AI: "); display.print(akurasi, 0); display.println("%");
  } else {
    display.println("Slot tersedia");
  }

  display.display();
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_IN, OUTPUT);  pinMode(ECHO_IN, INPUT);
  pinMode(TRIG_OUT, OUTPUT); pinMode(ECHO_OUT, INPUT);
  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);

  gateServo.attach(SERVO_PIN);
  gateServo.write(0);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal diinisialisasi!");
    while (true) delay(1000);
  }

  initBuffer();
  updatePrediksiAI();
  updateLED();
  updateOLED();

  Serial.println("=== Smart Parking AI siap! ===");
}

// ============================================================
//  LOOP UTAMA
// ============================================================
unsigned long lastDisplayUpdate = 0;
int jumlahMobilSebelumnya = -1;

void loop() {
  unsigned long now = millis();

  // --- Update slot waktu (untuk moving average) setiap 5 menit ---
  if (now - lastSlotUpdate >= INTERVAL_SLOT_MS) {
    updateMovingAverage(slotWaktuAktif, jumlahMobil);
    evaluasiPrediksi();
    slotWaktuAktif = (slotWaktuAktif + 1) % TOTAL_SLOT_WAKTU;
    lastSlotUpdate = now;
  }

  // --- Baca kedua sensor ultrasonik ---
  long jarakMasuk  = bacaJarak(TRIG_IN, ECHO_IN);
  long jarakKeluar = bacaJarak(TRIG_OUT, ECHO_OUT);

  bool anomaliMasuk  = isAnomali(jarakMasuk,  rataJarakMasuk,  varJarakMasuk);
  bool anomaliKeluar = isAnomali(jarakKeluar, rataJarakKeluar, varJarakKeluar);

  if (jumlahMobil != jumlahMobilSebelumnya) {
    Serial.printf("[EVENT] Mobil:%d Sisa:%d Status:%s\n",
      jumlahMobil, KAPASITAS_MAKS - jumlahMobil, labelNama[labelAktif]);
    jumlahMobilSebelumnya = jumlahMobil;
  }

  // --- Mobil Masuk ---
  if (!anomaliMasuk && jarakMasuk <= JARAK_TRIGGER_CM &&
      (now - lastEventMasuk > COOLDOWN_MS)) {

    if (jumlahMobil < KAPASITAS_MAKS) {
      jumlahMobil++;
      lastEventMasuk = now;
      updateMovingAverage(slotWaktuAktif, jumlahMobil);
      updatePrediksiAI();
      bukaPalang();
      updateLED();
      updateOLED();
      // tunggu mobil benar-benar lewat sebelum lanjut
      while (bacaJarak(TRIG_IN, ECHO_IN) <= JARAK_TRIGGER_CM) delay(100);
      delay(1000);
    }
  }
  // --- Mobil Keluar ---
  else if (!anomaliKeluar && jarakKeluar <= JARAK_TRIGGER_CM &&
           (now - lastEventKeluar > COOLDOWN_MS) && jumlahMobil > 0) {

    jumlahMobil--;
    lastEventKeluar = now;
    updateMovingAverage(slotWaktuAktif, jumlahMobil);
    updatePrediksiAI();
    updateLED();
    updateOLED();
    while (bacaJarak(TRIG_OUT, ECHO_OUT) <= JARAK_TRIGGER_CM) delay(100);
    delay(1000);
  }

  // --- Refresh OLED berkala (1x/detik) walau tidak ada event ---
  if (now - lastDisplayUpdate >= 1000) {
    updateOLED();
    lastDisplayUpdate = now;
  }

  delay(100);
}

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include <DHT.h>
#include <SoftwareSerial.h>
SoftwareSerial espSerial(10, 11); // RX, TX
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#define TANAH_KANAN A0
#define TANAH_KIRI  A1
#define POMPA_KANAN 7
#define POMPA_KIRI  8

// Kalibrasi
int nilaiKering = 1020;
int nilaiBasah = 400;
unsigned long waktuMulaiKanan = 0;
unsigned long waktuMulaiKiri = 0;
bool pompaKananAktif = false;
bool pompaKiriAktif = false;
const unsigned long durasiPompa = 5000;  // 5 detik
const unsigned long jedaPompa = 10000;   // 30 detik
unsigned long waktuSelesaiKanan = 0;
unsigned long waktuSelesaiKiri = 0;

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  lcd.init();
  lcd.backlight();
  pinMode(POMPA_KANAN, OUTPUT);
  pinMode(POMPA_KIRI, OUTPUT);
  digitalWrite(POMPA_KANAN, HIGH);  // Matikan pompa awal
  digitalWrite(POMPA_KIRI, HIGH);
  dht.begin();
  Serial.println("Sistem Arduino siap!");
}
int hitungPersen(int nilai) {
  return constrain(map(nilai, nilaiKering, nilaiBasah, 0, 100), 0, 100);
}

void loop() {
  // Baca sensor
  int tanahKanan = hitungPersen(analogRead(TANAH_KANAN));
  int tanahKiri = hitungPersen(analogRead(TANAH_KIRI));
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();
  // Tampilkan di LCD
  lcd.setCursor(0, 1);
  lcd.print("R:");
  lcd.print(tanahKanan);
  lcd.print("% L:");
  lcd.print(tanahKiri);
  lcd.print("% ");
  lcd.setCursor(0, 0);
  lcd.print("Tem:");
  lcd.print(suhu, 1);
  lcd.print(" Hum:");
  lcd.print(kelembapan, 0);
  lcd.print("% ");
  // Kontrol otomatis pompa
  if (tanahKanan < 40 && !pompaKananAktif && (millis() - waktuSelesaiKanan > jedaPompa)) {
    digitalWrite(POMPA_KANAN, LOW);
    waktuMulaiKanan = millis();
    pompaKananAktif = true;
    Serial.println("Pompa kanan diaktifkan");
  }
  if (tanahKiri < 40 && !pompaKiriAktif && (millis() - waktuSelesaiKiri > jedaPompa)) {
    digitalWrite(POMPA_KIRI, LOW);
    waktuMulaiKiri = millis();
    pompaKiriAktif = true;
    Serial.println("Pompa kiri diaktifkan");
  }
  // Matikan pompa setelah durasi
  if (pompaKananAktif && millis() - waktuMulaiKanan >= durasiPompa) {
    digitalWrite(POMPA_KANAN, HIGH);
    pompaKananAktif = false;
    waktuSelesaiKanan = millis();
    Serial.println("Pompa kanan dimatikan");
  }
  if (pompaKiriAktif && millis() - waktuMulaiKiri >= durasiPompa) {
    digitalWrite(POMPA_KIRI, HIGH);
    pompaKiriAktif = false;
    waktuSelesaiKiri = millis();
    Serial.println("Pompa kiri dimatikan");
  }
  // Handle perintah dari ESP
  if (espSerial.available()) {
    String perintah = espSerial.readStringUntil('\n');
    perintah.trim();
    Serial.println("Perintah diterima: " + perintah);
    if (perintah == "REQ") {
      String balasan = "##START##\n";
      balasan += "💧 Tanah Kanan: " + String(tanahKanan) + "%\n";
      balasan += "💧 Tanah Kiri: " + String(tanahKiri) + "%\n";
      balasan += "🌡 Suhu: " + String(suhu, 1) + "°C\n";
      balasan += "💦 Kelembapan: " + String(kelembapan, 0) + "%\n";
      balasan += "🚿 Pompa Kanan: " + String(pompaKananAktif ? "HIDUP" : "MATI") + "\n";
      balasan += "🚿 Pompa Kiri: " + String(pompaKiriAktif ? "HIDUP" : "MATI") + "\n";
      balasan += "##END##"; // Tanda akhir data
      espSerial.println(balasan);
      Serial.println("Mengirim: " + balasan);
      delay(100);
    }
    else if (perintah == "NYALA") {
      digitalWrite(POMPA_KANAN, LOW);
      digitalWrite(POMPA_KIRI, LOW);
      waktuMulaiKanan = millis();
      waktuMulaiKiri = millis();
      pompaKananAktif = true;
      pompaKiriAktif = true;
      espSerial.println("##START##\nSemua pompa dihidupkan selama 5 detik!\n##END##");
    }
    else if (perintah == "MATI") {
      digitalWrite(POMPA_KANAN, HIGH);
      digitalWrite(POMPA_KIRI, HIGH);
      pompaKananAktif = false;
      pompaKiriAktif = false;
      espSerial.println("##START##\nSemua pompa dimatikan secara manual\n##END##");
    } 
  }
  delay(500);
}
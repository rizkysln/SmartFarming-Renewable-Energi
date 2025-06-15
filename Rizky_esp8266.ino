#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
const char* ssid = "SmartTani_Bot";
const char* pass = "12345678";
const char* botToken = "7979079631:AAGSn53-Gsbbcus96ZmeSiNkgM10zqCNsC0";
const int64_t chatID = 5383352245;
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);
SoftwareSerial unoSerial(D1, D2);  // RX, TX
unsigned long lastCheck = 0;
const unsigned long checkInterval = 3000;

void setup() {
  Serial.begin(9600);
  unoSerial.begin(9600);
  client.setInsecure();
  WiFi.begin(ssid, pass);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung. IP: " + WiFi.localIP().toString());
  bot.sendMessage(String(chatID), "🤖 *Bot Siap Digunakan!*", "Markdown");
}

void loop() {
  // Cek pesan masuk dari Telegram
  if (millis() - lastCheck > checkInterval) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("📩 Pesan baru diterima!");
      handleMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastCheck = millis();
  }
}

void handleMessages(int total) {
  for (int i = 0; i < total; i++) {
    String pesan = bot.messages[i].text;
    String idPengirim = bot.messages[i].chat_id;

    if (pesan == "/start") {
      bot.sendMessage(idPengirim, "Halo! Kirim /cek untuk melihat data sensor. 🌱", "");
    }

    else if (pesan == "/cek") {
      Serial.println("📤 Mengirim REQ ke Arduino...");
      unoSerial.println("REQ");  // Kirim perintah ke Arduino

      String dataBalik = bacaDariUno(); // Baca respon dari Arduino
      bot.sendMessage(idPengirim, dataBalik, "Markdown");
    }
  }
}

// Fungsi baca data dari Arduino
String bacaDariUno() {
  String buffer = "";
  bool mulai = false;
  unsigned long timeout = millis();
  while (millis() - timeout < 3000) {
    if (unoSerial.available()) {
      String baris = unoSerial.readStringUntil('\n');
      baris.trim();
      Serial.println("[ESP] Terima: " + baris);

      if (baris == "##START##") {
        mulai = true;
        buffer = "";
      } else if (baris == "##END##") {
        break;
      } else if (mulai) {
        buffer += baris + "\n";
      }
    }
    yield();  // Cegah reset watchdog
  }
  if (buffer == "") buffer = "❗ Tidak ada respon dari Arduino.";
  return buffer;
}

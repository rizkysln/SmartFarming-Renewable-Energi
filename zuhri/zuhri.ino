#include <Wire.h>
#include <Adafruit_INA219.h>
#include <LiquidCrystal_I2C.h>

Adafruit_INA219 ina219;
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define VOLTAGE_SENSOR_PIN A0
float R1 = 30000.0; // Resistor 30k ohm
float R2 = 7500.0;  // Resistor 7.5k ohm
float calibrationFactor = 12.0 / 12.88; // Faktor kalibrasi berdasarkan pengukuran nyata

void setup() {
    Serial.begin(115200);
    if (!ina219.begin()) {
        Serial.println("Gagal menemukan INA219, cek koneksi!");
        while (1);
    }
    Serial.println("INA219 siap digunakan");
    
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Sensor INA219");
}

void loop() {
    float busVoltage = ina219.getBusVoltage_V();   // Membaca tegangan bus (V)
    float shuntVoltage = ina219.getShuntVoltage_mV() / 1000; // Membaca tegangan shunt (V)
    float current_mA = ina219.getCurrent_mA();     // Membaca arus (mA)
    float power_mW = ina219.getPower_mW();         // Membaca daya (mW)
    
    int sensorValue = analogRead(VOLTAGE_SENSOR_PIN);
    float voltage = (sensorValue * 5.0 / 1023.0) * ((R1 + R2) / R2) * calibrationFactor; // Kalibrasi hasil pembacaan
    
    Serial.print("Tegangan Bus: "); Serial.print(busVoltage); Serial.println(" V");
    Serial.print("Tegangan Shunt: "); Serial.print(shuntVoltage); Serial.println(" V");
    Serial.print("Arus: "); Serial.print(current_mA); Serial.println(" mA");
    Serial.print("Daya: "); Serial.print(power_mW); Serial.println(" mW");
    Serial.print("Tegangan: "); Serial.print(voltage); Serial.println(" V");
    Serial.println("------------------------------------");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("V: "); lcd.print(voltage, 2); lcd.print("V");
    lcd.setCursor(0, 1);
    lcd.print("I: "); lcd.print(current_mA, 2); lcd.print("mA");
    
    delay(1000); // Delay 1 detik sebelum pembacaan berikutnya
}

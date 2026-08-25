#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// =====================================================
//                     WIFI
// =====================================================

#define WIFI_SSID "TestWifi"
#define WIFI_PASSWORD "12345678"

// =====================================================
//                   FIREBASE
// =====================================================

#define API_KEY "AIzaSyDwunyUMUm4QuqGhY5RUcbVqJJ2ZiPq50g"

#define DATABASE_URL "https://milk-analyzer-8d50e-default-rtdb.asia-southeast1.firebasedatabase.app"

// Firebase Authentication
#define FIREBASE_EMAIL "smartmilkanalyzer@gmail.com"
#define FIREBASE_PASSWORD "Mudasir37@_#"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// =====================================================
//                  PIN DEFINITIONS
// =====================================================

// HX711
#define HX711_DT 4
#define HX711_SCK 5

// DS18B20
#define ONE_WIRE_BUS 15

// TDS module
#define TDS_PIN 34

// Photodiode
#define PHOTODIODE_PIN 35

// =====================================================
//                     LCD
// =====================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);

// =====================================================
//                    HX711
// =====================================================

HX711 scale;

float calibration_factor = -420.7;

// =====================================================
//                  DS18B20
// =====================================================

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

// =====================================================
//                  VARIABLES
// =====================================================

float milkWeight = 0;
float temperature = 0;
float tds = 0;

int photoValue = 0;

float fat = 0;
float snf = 0;
float waterPercent = 0;
float price = 0;

// Base price
float basePricePerKg = 50.0;

// =====================================================
//                     SETUP
// =====================================================

void setup() {

  // ---------------------------------------------------
  // SERIAL
  // ---------------------------------------------------

  Serial.begin(115200);
  delay(1000);

  // ---------------------------------------------------
  // I2C
  // ---------------------------------------------------

  Wire.begin(21, 22);

  // ---------------------------------------------------
  // LCD
  // ---------------------------------------------------

  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MILK ANALYZER");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  delay(2000);

  // ---------------------------------------------------
  // TEMPERATURE SENSOR
  // ---------------------------------------------------

  tempSensor.begin();

  // ---------------------------------------------------
  // LOAD CELL
  // ---------------------------------------------------

  scale.begin(HX711_DT, HX711_SCK);
  scale.set_scale(calibration_factor);

  // ---------------------------------------------------
  // TARE
  // ---------------------------------------------------

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TARING...");
  lcd.setCursor(0, 1);
  lcd.print("Empty Container");

  Serial.println();
  Serial.println("================================");
  Serial.println("       MILK ANALYZER");
  Serial.println("================================");
  Serial.println("Keep EMPTY container on scale.");

  delay(3000);

  scale.tare();

  Serial.println("Tare complete.");
  Serial.println("Container = 0 g");

  // ===================================================
  // WIFI CONNECTION
  // ===================================================

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecting");

  Serial.println();
  Serial.println("Connecting to WiFi...");
  Serial.print("Network: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 30) {

    delay(500);
    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WiFi CONNECTED!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");

    delay(1000);

  } else {

    Serial.println("WiFi FAILED!");
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi FAILED");

    while (true) {
      delay(1000);
    }
  }

  // ===================================================
  // FIREBASE
  // ===================================================

  config.api_key = API_KEY;
config.database_url = DATABASE_URL;

// Firebase Authentication
auth.user.email = FIREBASE_EMAIL;
auth.user.password = FIREBASE_PASSWORD;

Serial.println("Starting Firebase...");

Firebase.begin(&config, &auth);

Serial.println("Firebase.begin finished!");

Serial.println("Firebase.begin finished!");

Firebase.reconnectWiFi(true);

Serial.println("Firebase initialized.");

// Check authentication
Serial.println("Checking Firebase...");

int firebaseAttempts = 0;

while (!Firebase.ready() && firebaseAttempts < 20) {
  delay(500);
  Serial.print(".");
  firebaseAttempts++;
}

Serial.println();

if (Firebase.ready()) {
  Serial.println("Firebase READY!");
} else {
  Serial.println("Firebase authentication failed!");
  Serial.println("Check Firebase email/password.");
}

  // ===================================================
  // READY
  // ===================================================

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("READY");
  lcd.setCursor(0, 1);
  lcd.print("Type T to Test");

  Serial.println();
  Serial.println("================================");
  Serial.println("          SYSTEM READY");
  Serial.println("================================");
  Serial.println("1. Add milk to container.");
  Serial.println("2. Dip required sensors.");
  Serial.println("3. Type T in Serial Monitor.");
  Serial.println("4. Press Enter/Send.");
  Serial.println("================================");
}

// =====================================================
//              READ TEMPERATURE
// =====================================================

float readTemperature() {

  tempSensor.requestTemperatures();

  float t = tempSensor.getTempCByIndex(0);

  if (t == DEVICE_DISCONNECTED_C || t == -127.0) {

    Serial.println("Temperature sensor error.");

    // Keep test running
    return 25.0;
  }

  return t;
}

// =====================================================
//                    READ TDS
// =====================================================

float readTDS(float temperatureValue) {

  int raw = analogRead(TDS_PIN);

  Serial.print("TDS Raw: ");
  Serial.println(raw);

  float voltage =
    raw * 3.3 / 4095.0;

  float compensation =
    1.0 + 0.02 * (temperatureValue - 25.0);

  float compensatedVoltage =
    voltage / compensation;

  float tdsValue =
    (133.42 *
     compensatedVoltage *
     compensatedVoltage *
     compensatedVoltage
     -
     255.86 *
     compensatedVoltage *
     compensatedVoltage
     +
     857.39 *
     compensatedVoltage) * 0.5;

  if (tdsValue < 0) {
    tdsValue = 0;
  }

  return tdsValue;
}

// =====================================================
//                READ PHOTODIODE
// =====================================================

int readPhotodiode() {

  int value =
    analogRead(PHOTODIODE_PIN);

  Serial.print("Photodiode: ");
  Serial.println(value);

  return value;
}

// =====================================================
//                  FAT ESTIMATION
// =====================================================

float calculateFat(int photo) {

  float result =
    ((float)photo / 4095.0) * 8.0;

  if (result < 0) {
    result = 0;
  }

  if (result > 8) {
    result = 8;
  }

  return result;
}

// =====================================================
//                  WATER ESTIMATION
// =====================================================

float calculateWater(float tdsValue) {

  float water = 0;

  if (tdsValue < 600) {
    water = 20;
  }
  else if (tdsValue < 650) {
    water = 10;
  }
  else {
    water = 0;
  }

  return water;
}

// =====================================================
//                       SNF
// =====================================================

float calculateSNF(
  float fatValue,
  float waterValue) {

  float result = 8.5;

  result +=
    (fatValue - 4.0) * 0.15;

  result -=
    waterValue * 0.05;

  if (result < 5.0) {
    result = 5.0;
  }

  if (result > 10.0) {
    result = 10.0;
  }

  return result;
}

// =====================================================
//                      PRICE
// =====================================================

float calculatePrice(
  float fatValue,
  float snfValue) {

  float finalPrice =
    basePricePerKg;

  if (fatValue >= 4.0) {
    finalPrice += 5.0;
  }

  if (snfValue >= 8.5) {
    finalPrice += 8.0;
  }

  return finalPrice;
}

// =====================================================
//                CHECK FOR SAMPLE
// =====================================================

bool samplePresent(
  float weight,
  float tdsValue) {

  if (weight > 5) {
    return true;
  }

  if (tdsValue > 20) {
    return true;
  }

  return false;
}

// =====================================================
//                FIREBASE UPLOAD
// =====================================================

bool uploadToFirebase() {

  Serial.println();
  Serial.println("Uploading to Firebase...");

  bool success = true;

  // ---------------------------------------------------
  // FAT
  // Firebase: /readings/fat
  // ---------------------------------------------------

  if (Firebase.RTDB.setFloat(
        &fbdo,
        "/readings/fat",
        fat)) {

    Serial.println("Fat uploaded.");

  } else {

    success = false;

    Serial.print("Fat upload error: ");
    Serial.println(fbdo.errorReason());
  }

  // ---------------------------------------------------
  // SNF
  // Firebase: /readings/snf
  // ---------------------------------------------------

  if (Firebase.RTDB.setFloat(
        &fbdo,
        "/readings/snf",
        snf)) {

    Serial.println("SNF uploaded.");

  } else {

    success = false;

    Serial.print("SNF upload error: ");
    Serial.println(fbdo.errorReason());
  }

  // ---------------------------------------------------
  // WATER
  // Firebase: /readings/water
  // ---------------------------------------------------

  String waterStatus;

  if (waterPercent > 0) {
    waterStatus = "Detected";
  } else {
    waterStatus = "Not Detected";
  }

  if (Firebase.RTDB.setString(
        &fbdo,
        "/readings/water",
        waterStatus)) {

    Serial.println("Water uploaded.");

  } else {

    success = false;

    Serial.print("Water upload error: ");
    Serial.println(fbdo.errorReason());
  }

  // ---------------------------------------------------
  // PRICE
  // Firebase: /readings/price
  // ---------------------------------------------------

  if (Firebase.RTDB.setFloat(
        &fbdo,
        "/readings/price",
        price)) {

    Serial.println("Price uploaded.");

  } else {

    success = false;

    Serial.print("Price upload error: ");
    Serial.println(fbdo.errorReason());
  }

  // ---------------------------------------------------
  // TEMPERATURE
  // Firebase: /readings/temperature
  // ---------------------------------------------------

  if (Firebase.RTDB.setFloat(
        &fbdo,
        "/readings/temperature",
        temperature)) {

    Serial.println("Temperature uploaded.");

  } else {

    success = false;

    Serial.print("Temperature upload error: ");
    Serial.println(fbdo.errorReason());
  }

  // ---------------------------------------------------
  // FINAL UPLOAD STATUS
  // ---------------------------------------------------

  if (success) {

    Serial.println();
    Serial.println("================================");
    Serial.println("FIREBASE UPLOAD SUCCESSFUL!");
    Serial.println("================================");

  } else {

    Serial.println();
    Serial.println("================================");
    Serial.println("FIREBASE UPLOAD FAILED!");
    Serial.println("================================");
  }

  return success;
}

// =====================================================
//              TAKE ONE COMPLETE TEST
// =====================================================

void performTest() {

  Serial.println();
  Serial.println("================================");
  Serial.println("          TEST STARTED");
  Serial.println("================================");

  // ---------------------------------------------------
  // TESTING SCREEN
  // ---------------------------------------------------

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Testing...");
  lcd.setCursor(0, 1);
  lcd.print("Please wait");

  delay(1500);

  // ---------------------------------------------------
  // LOAD CELL
  // ---------------------------------------------------

  milkWeight =
    scale.get_units(20);

  if (milkWeight < 0) {
    milkWeight = 0;
  }

  if (milkWeight < 2) {
    milkWeight = 0;
  }

  Serial.print("Weight: ");
  Serial.println(milkWeight);

  // ---------------------------------------------------
  // TEMPERATURE
  // ---------------------------------------------------

  temperature =
    readTemperature();

  Serial.print("Temperature: ");
  Serial.println(temperature);

  // ---------------------------------------------------
  // TDS
  // ---------------------------------------------------

  tds =
    readTDS(temperature);

  Serial.print("TDS: ");
  Serial.println(tds);

  // ---------------------------------------------------
  // PHOTODIODE
  // ---------------------------------------------------

  photoValue =
    readPhotodiode();

  // ---------------------------------------------------
  // CHECK SAMPLE
  // ---------------------------------------------------

  if (!samplePresent(
        milkWeight,
        tds)) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("NO SAMPLE");
    lcd.setCursor(0, 1);
    lcd.print("Add Milk");

    Serial.println();
    Serial.println("NO SAMPLE DETECTED.");

    delay(3000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("READY");
    lcd.setCursor(0, 1);
    lcd.print("Type T to Test");

    return;
  }

  // ---------------------------------------------------
  // CALCULATE FAT
  // ---------------------------------------------------

  fat =
    calculateFat(photoValue);

  // ---------------------------------------------------
  // CALCULATE WATER
  // ---------------------------------------------------

  waterPercent =
    calculateWater(tds);

  // ---------------------------------------------------
  // CALCULATE SNF
  // ---------------------------------------------------

  snf =
    calculateSNF(
      fat,
      waterPercent);

  // ---------------------------------------------------
  // CALCULATE PRICE
  // ---------------------------------------------------

  price =
    calculatePrice(
      fat,
      snf);

  // ===================================================
  // FINAL RESULT
  // ===================================================

  Serial.println();
  Serial.println("================================");
  Serial.println("       MILK QUALITY RESULT");
  Serial.println("================================");

  Serial.print("Fat               : ");
  Serial.print(fat, 1);
  Serial.println(" %");

  Serial.print("SNF               : ");
  Serial.print(snf, 1);
  Serial.println(" %");

  Serial.print("Water Adulteration: ");

  if (waterPercent > 0) {
    Serial.print(waterPercent, 1);
    Serial.println(" %");
  } else {
    Serial.println("Not Detected");
  }

  Serial.print("Price             : Rs ");
  Serial.print(price, 2);
  Serial.println("/kg");

  Serial.println("================================");

  // ===================================================
  // UPLOAD TO FIREBASE
  // ===================================================

  bool firebaseSuccess =
    uploadToFirebase();

  // ===================================================
  // LCD RESULTS
  // ===================================================

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("FAT:");
  lcd.print(fat, 1);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("SNF:");
  lcd.print(snf, 1);
  lcd.print("%");

  delay(2500);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("WATER:");

  if (waterPercent > 0) {
    lcd.print(waterPercent, 1);
    lcd.print("%");
  } else {
    lcd.print("NO");
  }

  lcd.setCursor(0, 1);
  lcd.print("PRICE:");

  delay(2000);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("PRICE:");

  lcd.setCursor(0, 1);
  lcd.print("Rs ");
  lcd.print(price, 2);
  lcd.print("/kg");

  delay(3000);

  // ---------------------------------------------------
  // FINAL STATUS
  // ---------------------------------------------------

  lcd.clear();

  if (firebaseSuccess) {

    lcd.setCursor(0, 0);
    lcd.print("DATA UPLOADED");

    lcd.setCursor(0, 1);
    lcd.print("Website Ready");

    Serial.println();
    Serial.println("DATA SUCCESSFULLY UPLOADED.");
    Serial.println("Website can now read the data.");

  } else {

    lcd.setCursor(0, 0);
    lcd.print("UPLOAD FAILED");

    lcd.setCursor(0, 1);
    lcd.print("Check Firebase");

    Serial.println();
    Serial.println("DATA WAS NOT UPLOADED.");
  }

  delay(3000);

  // ---------------------------------------------------
  // READY FOR NEXT TEST
  // ---------------------------------------------------

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("READY");

  lcd.setCursor(0, 1);
  lcd.print("Type T to Test");

  Serial.println();
  Serial.println("SYSTEM READY");
  Serial.println("Type T for next test.");
}

// =====================================================
//                    MAIN LOOP
// =====================================================

void loop() {

  // One test happens only when T is entered.

  if (Serial.available()) {

    char command =
      Serial.read();

    if (command == 'T' ||
        command == 't') {

      performTest();
    }
  }
}
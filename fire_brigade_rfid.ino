#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define SS_PIN 5
#define RST_PIN 22

#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

#define API_KEY "API_KEY"
#define DATABASE_URL "DATABASE_URL"
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

// STATION location
// This prototype uses only "one RFID reader" (at the fire station) and "two RFID tags" (representing two fire trucks).
// In the final system, there will be multiple RFID checkpoints for tracking, but for now, we are only implementing the station tracking.
#define STATION_LAT 12.3456
#define STATION_LON 78.9012
#define STATION_ID "station1"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

MFRC522 rfid(SS_PIN, RST_PIN);
String lastCardUID = "";
unsigned long lastScanTime = 0;
const unsigned long SCAN_DELAY = 3000;

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  SPI.begin();
  rfid.PCD_Init();
  
  String stationPath = "/fire_trucks/" + String(STATION_ID) + "/available_trucks";
  if (!Firebase.RTDB.getInt(&fbdo, stationPath)) {
    Firebase.RTDB.setInt(&fbdo, stationPath, 0);
  }

  Serial.println("STATION RFID Scanner Ready");
}

bool updateTruckStatus(String truckId) {
  String scanCountPath = "/truck_status/" + truckId + "/scan_count";
  int scanCount = 0;
  
  if (Firebase.RTDB.getInt(&fbdo, scanCountPath)) {
    scanCount = fbdo.intData();
  }
  
  scanCount++;
  
  // Determine if truck is in station (even count) or out (odd count)
  bool isInStation = (scanCount % 2 == 0);
  
  FirebaseJson json;
  json.set("truck_id", truckId);
  json.set("station_id", STATION_ID);
  json.set("latitude", STATION_LAT);
  json.set("longitude", STATION_LON);
  json.set("timestamp/.sv", "timestamp");
  json.set("scan_count", scanCount);
  json.set("status", isInStation ? "in_station" : "out_of_station");

  // Update truck's current status
  String statusPath = "/truck_status/" + truckId;
  if (!Firebase.RTDB.setJSON(&fbdo, statusPath, &json)) {
    Serial.println("Failed to update truck status");
    Serial.println(fbdo.errorReason());
    return false;
  }

  // Get current available trucks count
  String stationPath = "/fire_trucks/" + String(STATION_ID) + "/available_trucks";
  int currentTrucks = 0;
  
  if (Firebase.RTDB.getInt(&fbdo, stationPath)) {
    currentTrucks = fbdo.intData();
  }
  
  // Update available trucks count
  int newTruckCount = isInStation ? currentTrucks + 1 : max(0, currentTrucks - 1);
  if (!Firebase.RTDB.setInt(&fbdo, stationPath, newTruckCount)) {
    Serial.println("Failed to update available trucks count");
    Serial.println(fbdo.errorReason());
    return false;
  }

  String historyPath = "/scan_history/" + truckId + "/" + String(millis());
  if (!Firebase.RTDB.setJSON(&fbdo, historyPath, &json)) {
    Serial.println("Failed to update scan history");
    Serial.println(fbdo.errorReason());
    return false;
  }

  Serial.println("\n--- Truck Update ---");
  Serial.print("Truck ID: ");
  Serial.println(truckId);
  Serial.print("Scan Count: ");
  Serial.println(scanCount);
  Serial.print("Status: ");
  Serial.println(isInStation ? "In Station" : "Out of Station");
  Serial.print("Available Trucks: ");
  Serial.println(newTruckCount);
  Serial.println("-----------------\n");

  return true;
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(5000);
    return;
  }

  if (!Firebase.ready()) {
    Serial.println("Firebase not ready. Reconnecting...");
    Firebase.begin(&config, &auth);
    delay(1000);
    return;
  }

  // Check for new card
  if (!rfid.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  
  // Read the card
  if (!rfid.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  if (millis() - lastScanTime < SCAN_DELAY) {
    rfid.PICC_HaltA();
    return;
  }

  // Get card UID
  String truckId = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      truckId += "0";
    }
    truckId += String(rfid.uid.uidByte[i], HEX);
  }
  truckId.toUpperCase();

  // Process new card
  if (truckId != lastCardUID || (millis() - lastScanTime >= SCAN_DELAY)) {
    lastCardUID = truckId;
    lastScanTime = millis();
    
    if (!updateTruckStatus(truckId)) {
      Serial.println("Error updating truck status. Will retry on next scan.");
    }
  }

  rfid.PICC_HaltA();
  delay(100);
}

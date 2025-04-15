#include <WiFi.h>
#include <HTTPClient.h>
#include <ThingSpeak.h>
#include <SPI.h>
#include <MFRC522.h>
#include <map>
// WiFi Credentials
const char* ssid = "your_ssid";
const char* password = "your_password";
// ThingSpeak Settings
const unsigned long CHANNEL_ID = your_channel_id;
const char* WRITE_API_KEY = "your_thingspeak_writeapikey";
const char* server = "api.thingspeak.com";
// RFID Settings
#define SS_PIN 21 // SDA (SS) pin connected to GPIO 21
#define RST_PIN 22 // RST pin connected to GPIO 22
MFRC522 mfrc522(SS_PIN, RST_PIN);
WiFiClient client;
// Map to store check-in status for each book
std::map<String, bool> bookStatusMap = {
  {"fcd3332", false}, // Book 1 with UID FCD33302, initially checked out
  {"a3d3a1d9", false}  // Book 2 with UID A3D3A1D9, initially checked out
};
void setup() {
  Serial.begin(115200);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
 }
  Serial.println("Connected to WiFi");
  // Initialize ThingSpeak
  ThingSpeak.begin(client);
  // Initialize RFID
  SPI.begin(18, 19, 23); // SCK -> GPIO 18, MISO -> GPIO 19, MOSI -> GPIO 23
  mfrc522.PCD_Init();
  Serial.println("Place RFID tag near the reader...");
}
void loop() {
  // Check for RFID Card
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.print("UID: ");
    Serial.println(uid);
    // Determine the check-in status of the book and update ThingSpeak
    if (uid == "fcd3332") {
      bookStatusMap[uid] = !bookStatusMap[uid]; // Toggle check-in status
      sendDataToThingSpeak(bookStatusMap[uid], 2); // Update Field 2 for Book 1
    } 
    else if (uid == "a3d3a1d9") {
      bookStatusMap[uid] = !bookStatusMap[uid]; // Toggle check-in status
      sendDataToThingSpeak(bookStatusMap[uid], 3); // Update Field 3 for Book 2
    }
    // Delay to prevent multiple reads
    delay(3000);
  }
  delay(5000); // Loop delay
}

// Function to Send Data to ThingSpeak
void sendDataToThingSpeak(bool isCheckedIn, int fieldNumber) {
  int statusValue = isCheckedIn ? 1 : 0; // 1 for checked in, 0 for checked out
  ThingSpeak.setField(fieldNumber, statusValue); // Update the appropriate field

  Serial.print("Sending data to ThingSpeak: Field ");
  Serial.print(fieldNumber);
  Serial.print(" -> ");
  Serial.println(statusValue);

  int responseCode = ThingSpeak.writeFields(CHANNEL_ID, WRITE_API_KEY);
  if (responseCode == 200) {
    Serial.println("Data sent successfully!");
  } else {
    Serial.print("Error sending data: ");
    Serial.println(responseCode);
  }
}
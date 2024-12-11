#include <ESP8266WiFi.h>
#include <Firebase.h>
#include <SoftwareSerial.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Wi-Fi credentials
#define WIFI_SSID "99"
#define WIFI_PASSWORD "11222211"

// Firebase configuration
#define FIREBASE_HOST "https://iot-database-00013781-default-rtdb.firebaseio.com/"
Firebase fb(FIREBASE_HOST);

// SoftwareSerial for Arduino communication
SoftwareSerial mySerial(D2, D1); // D2 as RX, D1 as TX

// NTP client setup
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 5 * 3600, 60000);  // Offset for Tashkent (UTC+5)

bool waitingMessageDisplayed = false;

void setup() {
  Serial.begin(9600);     // For debugging via USB
  delay(1000);            // Allow time for Serial Monitor to initialize
  mySerial.begin(9600);   // Communication with Arduino

  Serial.println("Starting NodeMCU setup...");

  connectWiFi(); // Connect to WiFi

  // Initialize NTP Client
  timeClient.begin();
  timeClient.update();  // Sync time immediately

  // Indicate readiness in Firebase
  Serial.println("Updating Firebase status...");
  if (fb.setString("MemoryGame/Status", "NodeMCU Ready")) {
    Serial.println("NodeMCU status set successfully in Firebase.");
  } else {
    Serial.println("Failed to update NodeMCU status in Firebase.");
  }
}

void loop() {
  if (mySerial.available()) {
    String data = mySerial.readStringUntil('\n'); // Read data from Arduino
    Serial.println("Received from Arduino: " + data); // Debug message

    if (data.startsWith("Level:")) {
      Serial.println("Valid data format detected. Processing...");
      processGameData(data); // Process and send to Firebase
    } else {
      Serial.println("Invalid data format received: " + data);
    }

    // Once data is received, hide the "Waiting for data" message
    waitingMessageDisplayed = false; 
  } else {
    if (!waitingMessageDisplayed) {
      Serial.println("Waiting for data from Arduino..."); // Display the message only once
      waitingMessageDisplayed = true; // Prevent repeated messages
    }
    delay(1000); // Prevent spamming the Serial Monitor
  }
}

void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
  } else {
    Serial.println("\nFailed to connect to WiFi. Check credentials or hardware.");
  }
}

void processGameData(String data) {
  Serial.println("Processing game data...");

  int levelIndex = data.indexOf("Level:") + 6;
  int scoreIndex = data.indexOf("Score:") + 6;

  if (levelIndex < 0 || scoreIndex < 0) {
    Serial.println("Malformed data received: " + data);
    return;
  }

  String levelStr = data.substring(levelIndex, data.indexOf(",", levelIndex));
  String scoreStr = data.substring(scoreIndex);

  int level = levelStr.toInt();
  int score = scoreStr.toInt();

  Serial.println("Extracted Data:");
  Serial.println("Level: " + String(level));
  Serial.println("Score: " + String(score));

  if (level <= 0 || score < 0) {
    Serial.println("Invalid level or score received.");
    return;
  }

  // Get the current real-time from NTP in Tashkent timezone (UTC+5)
  timeClient.update();
  String currentTime = timeClient.getFormattedTime(); // Get time in HH:MM:SS format

  Serial.println("Current time: " + currentTime); // Debug message

  // Construct the full path for this game session
  String fullPath = "MemoryGame/Players/" + String(millis());

  // Save the game data under this unique path, storing the time in HH:MM:SS format
  if (fb.setString(fullPath + "/Level", String(level)) &&
      fb.setString(fullPath + "/Score", String(score)) &&
      fb.setString(fullPath + "/Time", currentTime)) {
    Serial.println("New game data added successfully in Firebase.");
  } else {
    Serial.println("Failed to add new game data in Firebase.");
  }
}
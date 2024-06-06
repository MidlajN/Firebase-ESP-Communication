#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/RTDBHelper.h"
#include "addons/TokenHelper.h"

// Insert your network credentials
#define WIFI_SSID "WIFI_SSD"
#define WIFI_PASSWORD "PASSWORD"

// Insert Firebase project API Key and Database URL
#define API_KEY "FIREBASE_RTDB_API_KEY" 
#define DATABASE_URL "FIREBASE_RTDB_BASE_URL"

// Define Firebase Data object and authentication/configuration objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int LED_PIN = 2;
unsigned long sendDataPrevMills = 0; // Variable to store the previous time data was sent
int randon_number; // Variable to store the random number
bool led_status = false;
bool signUp = false; // Boolean to track the signup status

// Function to initialize Wi-Fi
void initWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Start Wi-Fi connection
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP()); // Print the IP address once connected
  Serial.println();
}

void setup() {
  Serial.begin(9600); // Initialize Serial for debugging
  pinMode(LED_PIN, OUTPUT);

  initWifi(); // Initialize Wi-Fi
  
  config.api_key = API_KEY; // Set the API key
  config.database_url = DATABASE_URL; // Set the database URL

  // Sign up anonymously
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("SignUp OK");
    signUp = true; // Set signUp to true if successful
  } else {
    Serial.printf("%s \n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; // Set the token status callback function

  Firebase.begin(&config, &auth); // Initialize Firebase with the config and auth
  Firebase.reconnectWiFi(true); // Enable automatic Wi-Fi reconnection
}

void loop() {

  // Check if Firebase is ready, the user has signed up, and 5 seconds have passed since the last data send
  if (Firebase.ready() && signUp && (millis() - sendDataPrevMills > 5000 || sendDataPrevMills == 0)) {
    sendDataPrevMills = millis(); // Update the time when data was last sent


    // -------------------------------- Set the value to the RTDB ------------------------------------

    randon_number = random(0, 1000) / 100.0; // Generate a random float number between 0 and 10
    // Write the data to Firebase Realtime Database at path "testRead"
    if (Firebase.RTDB.setFloat(&fbdo, "testRead", randon_number)) {
      Serial.println();
      Serial.print(randon_number); // Print the data
      Serial.print(" - Saved Successfully to : " + fbdo.dataPath()); // Print the path where data was saved
      Serial.print(" (" + fbdo.dataType() + ") "); // Print the data type
    } else {
      Serial.println("Failed : " + fbdo.errorReason()); // Print the error if data write fails
    }


    // --------------------- Read data from a RTDB to control devices attached to the esp32 -----------------------

    if (Firebase.RTDB.getBool(&fbdo, "/ledOn")) {
      if (fbdo.dataType() == "boolean") {
        led_status = fbdo.boolData();
        // Serial.println("SuccessFull READ from " + fbdo.dataPath() + " : " + led_status);
        if (led_status == 1) {
          digitalWrite(LED_PIN, HIGH);
        } else {
          digitalWrite(LED_PIN, LOW);
        }
      }
    } else {
      Serial.println("FAILED : " + fbdo.errorReason());
    }
  }
}

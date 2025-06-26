// master (control panel) with NFC and Long Range
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

#define SS_PIN    5    
#define RST_PIN   14   
#define DIO0_PIN  26   

#define SDA_PIN 21  
#define SCL_PIN 22   
Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

// storing the data
String userName = "";
String userAge = "";
String userDOB = "";
String userBloodGroup = "";


String cardUID = "";

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Master Initialization...");
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa Initialized. Waiting for NFC card...");
  Wire.begin(SDA_PIN, SCL_PIN);
  nfc.begin();
}

void loop() {
  uint8_t uid[7] = {0};
  uint8_t uidLength;

  // NFC card on master detection
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
    cardUID = "";
    for (uint8_t i = 0; i < uidLength; i++) {
      cardUID += String(uid[i], HEX);
    }

    Serial.println("NFC Card Detected on Master Side! UID: " + cardUID);

    // User input
    if (userName == "") {
      Serial.println("Please enter user data:");
      userName = readSerialInput("Enter Name: ");
      userAge = readSerialInput("Enter Age: ");
      userDOB = readSerialInput("Enter DOB: ");
      userBloodGroup = readSerialInput("Enter Blood Group: ");
    }

    // customizble input name, age DOB and blood group
    String message = cardUID + "," + userName + "," + userAge + "," + userDOB + "," + userBloodGroup;

    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    
    Serial.println("Data sent to slave: " + message);
    delay(2000);
  }

  // reciving the massage
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String receivedText = "";
    while (LoRa.available()) {
      receivedText += (char)LoRa.read();
    }

    Serial.println("Received from slave: " + receivedText);
  }
} 

String readSerialInput(String prompt) {
  Serial.println(prompt);
  while (Serial.available() == 0);
  return Serial.readStringUntil('\n');
}

// and another for slave of nfc 

// slave
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

#define SS_PIN    5    // LoRa module NSS (Chip select) pin
#define RST_PIN   14   // LoRa module RESET pin
#define DIO0_PIN  26   // LoRa module DIO0 pin

#define SDA_PIN 21  // NFC SDA Pin
#define SCL_PIN 22  // NFC SCL Pin
Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

String receivedUID = "";
String receivedName = "";
String receivedAge = "";
String receivedDOB = "";
String receivedBloodGroup = "";

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Slave Initialization...");
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa Initialized. Waiting for data...");
  Wire.begin(SDA_PIN, SCL_PIN);
  nfc.begin();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String receivedText = "";

    while (LoRa.available()) {
      receivedText += (char)LoRa.read();
    }

    Serial.println("Received from master: " + receivedText);

    // Split the received message into parts
    int delimiter1 = receivedText.indexOf(',');
    int delimiter2 = receivedText.indexOf(',', delimiter1 + 1);
    int delimiter3 = receivedText.indexOf(',', delimiter2 + 1);
    int delimiter4 = receivedText.indexOf(',', delimiter3 + 1);

    if (delimiter1 != -1 && delimiter2 != -1 && delimiter3 != -1 && delimiter4 != -1) {
      receivedUID = receivedText.substring(0, delimiter1);
      receivedName = receivedText.substring(delimiter1 + 1, delimiter2);
      receivedAge = receivedText.substring(delimiter2 + 1, delimiter3);
      receivedDOB = receivedText.substring(delimiter3 + 1, delimiter4);
      receivedBloodGroup = receivedText.substring(delimiter4 + 1);

      // Detect NFC card on the slave side
      uint8_t uid[7] = {0};
      uint8_t uidLength;

      if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        String cardUID = "";
        for (uint8_t i = 0; i < uidLength; i++) {
          cardUID += String(uid[i], HEX);
        }

        if (cardUID == receivedUID) {
          // Send the UID of the detected card back to the master
          String cardDetectedMessage = "Card Detected: " + cardUID;
          LoRa.beginPacket();
          LoRa.print(cardDetectedMessage);
          LoRa.endPacket();

          Serial.println("Sent to master: " + cardDetectedMessage);
        }
      }
    }
  }
}

// master code for OLED 

// master
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// LoRa Module Pins
#define SS_PIN    5    
#define RST_PIN   14   
#define DIO0_PIN  26   

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Master Initialization...");

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("OLED initialization failed!");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("LoRa Master");
  display.display();
  delay(1000);

  // Initialize LoRa
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa failed! Check wiring.");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("LoRa Failed!");
    display.display();
    while (1);
  }

  LoRa.setSyncWord(0xF3);  
  Serial.println("LoRa Ready!");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Waiting for Data...");
  display.display();
}

void loop() {
  int packetSize = LoRa.parsePacket();  

  if (packetSize) {
    String receivedText = "";
    
    while (LoRa.available()) {
      receivedText += (char)LoRa.read();
    }

    Serial.println("Received: " + receivedText);
    
    // Display on OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Received:");
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.print(receivedText);
    display.display();

    delay(2000); //  
  }
}

//  slave code for momentary switches
// slave
// pushh buttonLoRa
#include <SPI.h>
#include <LoRa.h>

#define SS_PIN    5   
#define RST_PIN   14  
#define DIO0_PIN  26  

#define SWITCH_A 32  
#define SWITCH_B 33  
#define SWITCH_C 25  
#define SWITCH_D 21  
#define SWITCH_E 27  

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Slave Initialization...");

  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed! Check wiring.");
    while (1);
  }

  LoRa.setSyncWord(0xF3);
  LoRa.setTxPower(17, PA_OUTPUT_PA_BOOST_PIN);

  Serial.println("LoRa Initialized. Ready to send data...");

  pinMode(SWITCH_A, INPUT_PULLUP);
  pinMode(SWITCH_B, INPUT_PULLUP);
  pinMode(SWITCH_C, INPUT_PULLUP);
  pinMode(SWITCH_D, INPUT_PULLUP);
  pinMode(SWITCH_E, INPUT_PULLUP);
}

void loop() {
  checkButtonPress(SWITCH_A, "Switch A: Need Medical Support");
  checkButtonPress(SWITCH_B, "Switch B: Need Food Support");
  checkButtonPress(SWITCH_C, "Switch C: Need Navigation Support");
  checkButtonPress(SWITCH_D, "Switch D: Need Other Support");
  checkButtonPress(SWITCH_E, "Switch E: No Need of Support");

  delay(100); // 
}


void checkButtonPress(int buttonPin, String message) {
  static unsigned long lastPressTime = 0;
  unsigned long currentTime = millis();

  int buttonState = digitalRead(buttonPin); 
  Serial.print("Button "); Serial.print(buttonPin); Serial.print(": "); Serial.println(buttonState);

  if (buttonState == LOW && (currentTime - lastPressTime) > 500) {  
    delay(50);  
    if (digitalRead(buttonPin) == LOW) {  
      sendLoRaMessage(message);
      Serial.println("Button Pressed: " + message);
      lastPressTime = currentTime;
      
      while (digitalRead(buttonPin) == LOW);  
      delay(200);  
    }
  }
}


void sendLoRaMessage(String message) {
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  Serial.println("Sent: " + message);
}



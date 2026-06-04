#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

#if defined(ESP32)   
  #include <WiFi.h>
#endif 

#include "firebase.h" 
#include "device.h" 

//  I/O for ESP32WROOM32
//  SDA: PIN 21
//  SCL: PIN 22

//Modem
const char* ssid = "MV007-F066"; 
const char* password = "64083282";

// //BSQ
// const char* ssid = "C 0707"; 
// const char* password = "12345678";



const int DELAY_BETWEEN_CARDS = 2000; // Increased to 2 seconds to prevent rapid double-tapping
long timeLastCardRead = 0;
boolean readerDisabled = false;
int irqCurr;
int irqPrev;

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

void handleCardDetected();
void startListeningToNFC();
void WifiConnect(); 

void setup() { 
  Serial.begin(115200); 
  
  pinMode(LED_BUILTIN, OUTPUT); 
  
  // Establish connection infrastructure
  WifiConnect(); 
  Firebase_Init("users"); 
  
  // Wake up and configure the PN532 module over I2C hardware channels
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) { 
    Serial.print("Critical Error: Didn't find PN53x hardware board link!"); 
    while (1); 
  } 
  
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  
  // Set up background reading parameters
  startListeningToNFC();
  Serial.println("System fully armed and ready."); 
} 

void loop() { 
  if (readerDisabled) { 
    if (millis() - timeLastCardRead > DELAY_BETWEEN_CARDS) { 
      readerDisabled = false; 
      startListeningToNFC(); 
    } 
  } else { 
    irqCurr = digitalRead(PN532_IRQ); 
    if (irqCurr == LOW && irqPrev == HIGH) { 
       Serial.println("Got NFC IRQ Edge Signal Trigger..."); 
       handleCardDetected(); 
    } 
    irqPrev = irqCurr; 
  } 
} 

void startListeningToNFC() { 
  irqPrev = irqCurr = HIGH; 
  nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A); 
} 

void handleCardDetected() { 
    uint8_t success = false; 
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  
    uint8_t uidLength; 
    
    success = nfc.readDetectedPassiveTargetID(uid, &uidLength); 
    
    if (success) { 
      Serial.println("Found card! Building data transmission payload..."); 
      
      // Parse byte arrays out into string elements directly matching keys
      String parsedUidStr = "";
      for (uint8_t i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) parsedUidStr += "0"; // Handle leading zero formatting rules
        parsedUidStr += String(uid[i], HEX);
      }
      parsedUidStr.toUpperCase(); // Ensure standard upper-case values ("A6560B05")
      
      Serial.print("Parsed Target UID String Token: ");
      Serial.println(parsedUidStr);
      
      // Dispatch tracking instructions out to the backend controller module
      LogCardTap(parsedUidStr);
      
      timeLastCardRead = millis(); 
    } 
    readerDisabled = true; 
} 

void WifiConnect() { 
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password); 
  while (WiFi.waitForConnectResult() != WL_CONNECTED) { 
    Serial.println("Connection Failed! Rebooting..."); 
    delay(5000); 
    ESP.restart(); 
  }   
  Serial.print("System connected with IP address: "); 
  Serial.println(WiFi.localIP()); 
}

void onFirebaseStream(FirebaseStream data) 
{ 
  Serial.printf("onFirebaseStream: %s %s %s %s\n", 
    data.streamPath().c_str(), 
    data.dataPath().c_str(), 
    data.dataType().c_str(), 
    data.stringData().c_str()); 
}
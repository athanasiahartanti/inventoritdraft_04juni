#include "firebase.h" 
#include "device.h" // Ensures pin mapping layout structures match architecture rules

// Authentication Targets linking microcontrollers to specific cloud instances
#define FIREBASE_HOST "https://iot-class08-athanasia-default-rtdb.firebaseio.com/" 
#define FIREBASE_AUTH "r5xHK5wrakthvdNJKQkuM2aNNzo9Xl0MfzFsO8tn" 

// Instantiate the technical protocol containers used by Mobizt Client Engine
FirebaseData fbdo;          // Standard structural database transaction channel container
FirebaseConfig fbConfig;    // Storage array containing targeting credentials 
FirebaseData fbdoStream;    // Independent connection slot dedicated ONLY to background streams

/**
 * Initializes the Firebase connection and binds a real-time event listener
 * @param streamPath The Firebase node folder path string to monitor (e.g., "cmd")
 */
void Firebase_Init(const String& streamPath) 
{ 
  FirebaseAuth fbAuth; // Storage structure block handling token/legacy validation rules
  
  // Assign configuration strings to system context properties
  fbConfig.host = FIREBASE_HOST; 
  fbConfig.signer.tokens.legacy_token = FIREBASE_AUTH; // Attaches the database secret key token
  
  // Initialize communication utilizing target credentials and structural contexts
  Firebase.begin(&fbConfig, &fbAuth); 
  
  // Keep connection resilient: Re-negotiate network routing automatically if Wi-Fi blips
  Firebase.reconnectWiFi(true); 

#if defined(ESP8266) 
  // ESP8266 BearSSL setting sizing tuning buffer limits to handle heavy handshakes smoothly
  fbdo.setBSSLBufferSize(2*1024, 1024); 
#endif 

  fbdo.setResponseSize(1024); // Restrict structural memory constraints for JSON buffering max allocations
  Firebase.RTDB.setwriteSizeLimit(&fbdo, "small"); // Optimization limiting structural block sizes
  
  // Halting state verification block waiting for active server communication link handshake
  while (!Firebase.ready()) 
  { 
    Serial.println("Connecting to firebase..."); 
    delay(1000); 
  } 

  String path = streamPath; 
  
  // Create an open socket listener block monitoring specific target node locations dynamically
  if (Firebase.RTDB.beginStream(&fbdoStream, path.c_str())) 
  { 
    Serial.println("Firebase stream on " + path);
    // Assign structural pointers referencing custom execution blocks when tracking events
    Firebase.RTDB.setStreamCallback(&fbdoStream, onFirebaseStream, onFirebaseStreamTimeout); 
  } 
  else {
    Serial.println("Firebase stream failed: " + fbdoStream.errorReason()); 
  }
} 

/**
 * Empty timeout verification interface handling broken websocket channels
 */
void onFirebaseStreamTimeout(bool timeout) 
{ 
  // Kept here as structural callback hook wrapper block
}

// Append this function to your existing firebase.cpp file


void LogCardTap(const String& cardUid) {
  // 1. Create a fast path pointing to our new quick-lookup index directory
  // 🌟 FIX: Updated to match your exact JSON folder key name: "cardIDtostudentNo"
  String lookupPath = "/cardIDtostudentNo/" + cardUid;
  String studentNo = "";

  Serial.print("Scanning lookup directory for Card: ");
  Serial.println(cardUid);

  // 2. Query the index folder to translate the Card UID into a Student Number (NIM)
  if (Firebase.RTDB.getString(&fbdo, lookupPath.c_str())) {
    if (fbdo.dataType() == "string") {
      studentNo = fbdo.stringData(); // We found the NIM! (e.g. "2802488990")
    }
  }

  // Fallback check: If the card isn't registered in our index book yet, reject it!
  if (studentNo == "") {
    Serial.printf("Access Denied! Card UID %s is not linked to any student number.\n", cardUid.c_str());
    return;
  }

  // 3. NOW PROCEED WITH YOUR EXACT SAME LOGIC, but using the studentNo as the folder key!
  String userPath = "/users/" + studentNo;
  
  if (Firebase.RTDB.getJSON(&fbdo, userPath.c_str())) {
    if (fbdo.dataType() == "json") {
      FirebaseJson &json = fbdo.to<FirebaseJson>();
      FirebaseJsonData studentNameData;
      FirebaseJsonData checkedInData;
      
      json.get(studentNameData, "studentName");
      json.get(checkedInData, "isStudentCheckedIn");
      
      bool currentStatus = checkedInData.boolValue;
      bool newStatus = !currentStatus; // Flip the bit!

      Serial.printf("Access Granted! Welcome back %s (NIM: %s)\n", 
                    studentNameData.stringValue.c_str(), 
                    studentNo.c_str());
      
      // Update the attendance boolean flag inside their NIM folder
      String statusPath = userPath + "/isStudentCheckedIn";
      Firebase.RTDB.setBool(&fbdo, statusPath.c_str(), newStatus);

      // 4. HISTORICAL LOGGING: Map it cleanly under the student's personal identification key
      String logPath = "/CheckoutHistory/log_" + studentNo + "_" + String(millis());
      
      FirebaseJson logData;
      // 🌟 FIX: Storing data matching your new blueprint field parameters
      logData.add("studentNo", studentNo); 
      logData.add("cardID", cardUid); // Changed from cardUid to cardID to stay fully consistent!
      logData.add("studentName", studentNameData.stringValue);
      logData.add("statusText", newStatus ? "Clocked In" : "Clocked Out");
      logData.add("timestamp", "{\".sv\": \"timestamp\"}"); // Using your cloud real-world time server stamp value!
      
      FirebaseJson timestampObj;
      timestampObj.set(".sv", "timestamp"); // This requests Google's actual server time
      logData.set("timestamp", timestampObj); // Inject it properly into your log map!
      
      if (Firebase.RTDB.setJSON(&fbdo, logPath.c_str(), &logData)) {
        Serial.println("Attendance event successfully synced with Server Timestamp.");
      }
    }
  }
}

//Previous Version (Full Made)
// void LogCardTap(const String& cardUid) {
//   String userPath = "/users/" + cardUid;
  
//   Serial.print("Checking database path: ");
//   Serial.println(userPath);

//   // 1. Read the full student JSON profile block from Firebase
//   if (Firebase.RTDB.getJSON(&fbdo, userPath.c_str())) {
//     if (fbdo.dataType() == "json") {
//       FirebaseJson &json = fbdo.to<FirebaseJson>();
//       FirebaseJsonData studentNameData;
//       FirebaseJsonData studentNoData;
//       FirebaseJsonData checkedInData;
      
//       // Extract metrics from the database node
//       json.get(studentNameData, "studentName");
//       json.get(studentNoData, "studentNo");
//       json.get(checkedInData, "isStudentCheckedIn"); // Get current status
      
//       // Handle fallback values if field doesn't exist yet
//       bool currentStatus = checkedInData.boolValue;
//       bool newStatus = !currentStatus; // Flip the bit!

//       Serial.printf("Access Granted! User Recognized: %s (%s)\n", 
//                     studentNameData.stringValue.c_str(), 
//                     studentNoData.stringValue.c_str());
      
//       // 2. TOGGLE LOGIC: Update the status flag right inside their profile folder
//       String statusPath = userPath + "/isStudentCheckedIn";
//       if (Firebase.RTDB.setBool(&fbdo, statusPath.c_str(), newStatus)) {
//         Serial.printf("Status updated to: %s\n", newStatus ? "INSIDE LAB" : "OUTSIDE LAB");
//       } else {
//         Serial.printf("Failed to update status flag: %s\n", fbdo.errorReason().c_str());
//       }

//       // 3. HISTORY LOGIC: Push structural row tracking log downstream
//       // Use a unique push key ID generator instead of millis() to avoid duplicate path naming collisions
//       String logPath = "/CheckoutHistory/log_" + cardUid + "_" + String(millis());
      
//       FirebaseJson logData;
//       logData.add("cardUid", cardUid);
//       logData.add("studentName", studentNameData.stringValue);
//       logData.add("studentNo", studentNoData.stringValue);
//       logData.add("statusText", newStatus ? "Clocked In" : "Clocked Out");
      
//       // 🌟 MAGIC LINE: Tells Firebase to insert its exact universal epoch timestamp value
//       logData.set("timestamp", "{\".sv\": \"timestamp\"}"); 
      
//       if (Firebase.RTDB.setJSON(&fbdo, logPath.c_str(), &logData)) {
//         Serial.println("Tap log successfully pushed to cloud /CheckoutHistory!");
//       } else {
//         Serial.printf("Failed to commit tracking log: %s\n", fbdo.errorReason().c_str());
//       }
      
//       // 3. HISTORY LOGIC: Push structural row tracking log downstream
//       // String logPath = "/CheckoutHistory/log_" + cardUid + "_" + String(millis());
      
//       // FirebaseJson logData;
//       // logData.add("cardUid", cardUid);
//       // logData.add("studentName", studentNameData.stringValue);
//       // logData.add("studentNo", studentNoData.stringValue);
//       // logData.add("statusText", newStatus ? "Clocked In" : "Clocked Out");
//       // logData.add("timestamp", String(millis())); 
      
//       // if (Firebase.RTDB.setJSON(&fbdo, logPath.c_str(), &logData)) {
//       //   Serial.println("Tap log successfully pushed to cloud /CheckoutHistory!");
//       // } else {
//       //   Serial.printf("Failed to commit tracking log: %s\n", fbdo.errorReason().c_str());
//       // }
//     }
//   } else {
//     // Fallback error response check
//     Serial.printf("Card UID %s unrecognized or database unreachable. Error: %s\n", 
//                   cardUid.c_str(), fbdo.errorReason().c_str());
//   }
// }


//THIS IS THE PREVIOUS VERSION (NO MADE)
// void LogCardTap(const String& cardUid) {
//   String userPath = "/users/" + cardUid;
  
//   Serial.print("Checking database path: ");
//   Serial.println(userPath);

//   // Read data from the targeted user slot
//   if (Firebase.RTDB.getJSON(&fbdo, userPath.c_str())) {
//     if (fbdo.dataType() == "json") {
//       FirebaseJson &json = fbdo.to<FirebaseJson>();
//       FirebaseJsonData studentNameData;
//       FirebaseJsonData studentNoData;
      
//       // Extract specific student metrics out of the JSON tree
//       json.get(studentNameData, "studentName");
//       json.get(studentNoData, "studentNo");
      
//       Serial.printf("Access Granted! User Recognized: %s (%s)\n", 
//                     studentNameData.stringValue.c_str(), 
//                     studentNoData.stringValue.c_str());
      
//       // Generate a dynamic log payload entry string using the card's UID
//       String logPath = "/CheckoutHistory/log_" + cardUid + "_" + String(millis());
      
//       FirebaseJson logData;
//       logData.add("cardUid", cardUid);
//       logData.add("studentName", studentNameData.stringValue);
//       logData.add("studentNo", studentNoData.stringValue);
//       logData.add("timestamp", String(millis())); // Fallback time tracking via runtime tracking clock
      
//       // Commit the transaction tracking map data upstream to Firebase
//       if (Firebase.RTDB.setJSON(&fbdo, logPath.c_str(), &logData)) {
//         Serial.println("Tap log successfully pushed to cloud database!");
//       } else {
//         Serial.printf("Failed to commit tracking log: %s\n", fbdo.errorReason().c_str());
//       }
//     }
//   } else {
//     // Handling case where unknown card is scanned or connection fails
//     Serial.printf("Card UID %s unrecognized or database unreachable. Error: %s\n", 
//                   cardUid.c_str(), fbdo.errorReason().c_str());
//   }
// }
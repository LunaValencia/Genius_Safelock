#include <SPI.h>
#include <SoftwareSerial.h>
#include <MFRC522.h>
#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
MFRC522::MIFARE_Key key;

const int buzzer = 7;
const int buzzerVcc = 8;
const int relay = 6;
const int Send = 3;
const int recieve = 4;
String userIDs[] = {"93:68:C3:C9", "CB:40:3D:2F", "C3:88:23:C9"};
int numUsers = 3;
const char* password = "12345678";
int failedAttempts = 0;

void setup() {
  SPI.begin();
  Serial.begin(9600);                                           // Initialize serial communications with the PC                                             // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card
  Serial.println(F("Read personal data on a MIFARE PICC:"));    //shows in serial that it is ready to read
  pinMode(buzzer, OUTPUT);
  pinMode(buzzerVcc, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(Send, OUTPUT);
  digitalWrite(Send, LOW);
  pinMode(2, OUTPUT);
}

void loop() {
  //////////////////////////////////////////RFID///////////////////////////////////////////
  //Serial.println(String(digitalRead(Send)) + String(digitalRead(recieve)));
  String userID = readRFID();

  if (userID.length() > 0) {  // Only check user if RFID was read successfully
    if (checkUser(userID)) {
      digitalWrite(buzzerVcc, HIGH); delay(70);
      digitalWrite(buzzerVcc, LOW);
      toggle(relay);
    }
    else {
      digitalWrite(buzzerVcc, HIGH); delay(70);
      digitalWrite(buzzerVcc, LOW);  delay(70);
      digitalWrite(buzzerVcc, HIGH); delay(70);
      digitalWrite(buzzerVcc, LOW);  delay(70);
      digitalWrite(buzzerVcc, HIGH); delay(70);
      digitalWrite(buzzerVcc, LOW);

      failedAttempts++;
      Serial.println(failedAttempts);
      if (failedAttempts == 3) {
        delay(300); digitalWrite(buzzerVcc, HIGH);
        delay(500); digitalWrite(buzzerVcc, LOW);
        digitalWrite(Send, HIGH); //Send to keypad+lcd
        Serial.println("SEND");
        Serial.println("Send to keypad+lcd");
        failedAttempts = 0;
        while (true) {
          if (digitalRead(recieve) == HIGH) { //if Keypad+lcd success
            Serial.println("RECIEVE");
            digitalWrite(Send, LOW);
            digitalWrite(buzzerVcc, HIGH); delay(70); digitalWrite(buzzerVcc, LOW);  delay(70);
            toggle(relay);
            break;
          }
        }
      }
    }
    if (digitalRead(A5) == HIGH) {
      while (true) {
        if (userID.length() > 0) {
          userID = userIDs[numUsers];
          numUsers++;
          break;
        }
      }
    }
  }
  if (digitalRead(recieve) == HIGH) { //if Keypad+lcd success
    Serial.println("RECIEVE");
    digitalWrite(Send, LOW);
    digitalWrite(buzzerVcc, HIGH); delay(70); digitalWrite(buzzerVcc, LOW);  delay(70);
    toggle(relay);
  }
}

void toggle(int Pin) {
  digitalWrite(Pin, HIGH);
  digitalWrite(2, HIGH);
  delay(3000);
  digitalWrite(Pin, LOW);
  digitalWrite(2, LOW);
}

bool checkUser(String userID) {
  // Check if user ID is in array
  bool found = false;
  for (int i = 0; i < numUsers; i++) {
    if (userID == userIDs[i]) {
      found = true;
      break;
    }
  }

  // Print result
  if (found) {
    Serial.println("Access granted");
    return true;
  } else {
    Serial.println("Access denied");
    return false;
  }
}

String readRFID() {
  MFRC522::MIFARE_Key key;

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  byte block;
  byte len;
  MFRC522::StatusCode status;
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    return "";  // Return an empty string if no card is present or if reading the card failed

  byte buffer1[18];

  String strID = "";
  for (byte i = 0 ; i < 4; i++) {

    strID += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") +
             String(mfrc522.uid.uidByte[i], HEX) +
             (i != 3 ? ":" : "");
  }
  strID.toUpperCase();
  Serial.println("Tap card key = " + strID);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  return strID;
}

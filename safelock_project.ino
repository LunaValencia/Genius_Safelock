#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
volatile int finger_status = -1;

SoftwareSerial mySerial(10, 11); // TX/RX on fingerprint sensor

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id = 1; // starting ID number
LiquidCrystal_I2C lcd(0x27,16,2);  
const byte ROWS = 4; 
const byte COLS = 4; 
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
const int recieve = 13;
const int Send = 12;
int numUsers = 1; 
const char* password = "12345678";
const int maxAttempts = 3;
int failedAttempts = 0;
int sec = 0;
int play = 1;
int reset = 0;
int keyStat = 0;

void setup() {
  Serial.begin(9600);                                           // Initialize serial communications with the PC
  Serial.println(F("Read personal data on a MIFARE PICC:"));    //shows in serial that it is ready to read
  pinMode(recieve,INPUT);
  pinMode(Send,OUTPUT);
  lcd.init();      
  lcd.backlight();               
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");
  lcd.clear();
  lcd.home();
  lcd.print("Tap your card...");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
}

void loop() {    
  //////////////////////////////////////////KEYPAD+LCD///////////////////////////////////////////
  //Serial.println(String(digitalRead(Send)) + ' ' + String(digitalRead(recieve)));
  if((digitalRead(recieve) == HIGH || keyStat == 1) && play == 1){    
     Serial.println("RECIEVE");
     digitalWrite(Send,LOW);
     lcd.clear();
     lcd.home();
     lcd.print("Enter PIN:");  
     play = 0;
  }
  else{
     digitalWrite(Send,LOW);
  }

  if (digitalRead(recieve) == HIGH || keyStat == 1){
    if(checkKeypad() == true){
      delay(1000);
      digitalWrite(Send,HIGH); //Send back
      digitalWrite(A3,LOW);
      Serial.println("SEND");
      lcd.clear();
      lcd.home();
      lcd.print("Tap your card...");
      play = 1;
      keyStat = 0;
    }
  }
  else{
    forceKeypad();
  }
}

void toggle(int Pin){
  if (digitalRead(Pin) == LOW){
      digitalWrite(Pin,HIGH);
    }
    else{
      digitalWrite(Pin,LOW);
    } 
}

bool forceKeypad() {
  char key = customKeypad.getKey();
  if (!key) {
    // No key pressed, return false
    return false;
  }
  if (key == '#'){
    keyStat = 1;
  }
}

bool checkKeypad() {  
  static char buffer[9] = {0};
  static int bufferPos = 0;
  char key = customKeypad.getKey();

  if (!key) {
    // No key pressed, return false
    return false;
  }

  if (!isdigit(key) && key != 'D' && key != '*') { // add check for new button
    // Invalid key, return false
    return false;
  }
  
  else if (key == 'D') { // handle password reset   
    if (reset == 0){ 
      resetPassword();
      reset++;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter PIN:");
      Serial.println("RESET");
    }
    else{
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("You have reached");
      lcd.setCursor(0, 1);
      lcd.print("max reset value");
      delay(800);
      lcd.setCursor(0, 0);
      lcd.print("Enter PIN:");
    }
  }

  if (bufferPos >= 8) {
    // Buffer is full, clear and return false
    memset(buffer, 0, sizeof(buffer));
    bufferPos = 0;
    return false;
  }

  if (key != '*' && key != 'D'){
    Serial.println(key);
    buffer[bufferPos++] = key;
    lcd.setCursor(bufferPos - 1, 1);
    lcd.print('*');
  }
  else{
    lcd.clear();
    buffer[9] = {0};
    bufferPos = 0;
    lcd.setCursor(0, 0);
    lcd.print("Enter PIN:");
    lcd.setCursor(0,1);
  }

  if (bufferPos < 8) {
    // Buffer not full yet, return false
    return false;
  }

  if (strcmp(buffer, password) == 0) {
    // Correct password entered
    memset(buffer, 0, sizeof(buffer));
    bufferPos = 0;
    failedAttempts = 0;
    lcd.clear();
    lcd.print("Access granted!");
    Serial.println("ACCESS");
    return true;
  }
    else {
    // Incorrect password entered
    memset(buffer, 0, sizeof(buffer));
    bufferPos = 0;
    failedAttempts++;
    lcd.clear();
    lcd.print("Access denied.");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter PIN:");
    //System locked
    if (failedAttempts == maxAttempts) {
      lcd.clear();
      lcd.print("System locked!");
      for (int sec = 60; sec > 0; sec--) {
        lcd.setCursor(3, 1);
        lcd.print("Seconds");
        lcd.setCursor(0, 1);
        if (sec <= 9){
          lcd.print(sec);
          lcd.print(" ");
          lcd.setCursor(2, 1);
          lcd.print("Seconds ");
          delay(1000);
        }
        else{
          lcd.print(sec);
          delay(1000);
        }
      }
      failedAttempts = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter PIN:");
    }
    return false;
  }
}

void resetPassword() {
  lcd.clear();
  lcd.home();
  lcd.print("Forget password?");
  lcd.setCursor(0,1);
  lcd.print("Verify finger.");
  while (true) { // keep waiting until a valid fingerprint is verified
      finger_status = getFingerprintIDez();
      if (finger_status!=-1 and finger_status!=-2){ //FINGER MATCHED
          Serial.println("Match");
          lcd.clear();
          lcd.print("Enter new PIN:");
          static char buffer[9] = {0};
          static int bufferPos = 0;
          while (bufferPos < 8) {
            char key = customKeypad.getKey();
            if (isdigit(key)) {
              buffer[bufferPos++] = key;
              lcd.setCursor(bufferPos - 1, 1);
              lcd.print('*');
            }
          }
          buffer[8] = '\0';
          password = buffer;
          Serial.println("New password is " + String(password));
          lcd.clear();
          lcd.print("New PIN saved!");
          delay(500);
          break;
      } else{ //FINGER NOT MATCHED
        if (finger_status==-2){
          for (int ii=0;ii<5;ii++){
            Serial.println("Not Match");
            lcd.clear();
            lcd.print("Fingerprint not");
            lcd.setCursor(0, 1);
            lcd.print("recognized");
            delay(500);
            lcd.clear();
            lcd.home();
            lcd.print("Forget password?");
            lcd.setCursor(0,1);
            lcd.print("Verify finger.");
          }
        }
      }
      delay(50);            //don't ned to run this at full speed.
  }
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p!=2){
    Serial.println(p);
  }
  if (p != FINGERPRINT_OK)  return -1;
  
  p = finger.image2Tz();
  if (p!=2){
    Serial.println(p);
  }
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -2;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}

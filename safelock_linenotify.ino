#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <TridentTD_LineNotify.h>

#define Data D6
#define SSID        "gett"   // ชื่อ Wifi
#define PASSWORD    "123456789"   // รหัส Wifi
#define LINE_TOKEN  "XSpdl8UJY7TLRsD2E7iLBZ2fgix5xoVP9siOJqf7dJs"   // TOKEN

void setup() {
  pinMode(Data,INPUT);
  Serial.begin(115200);
  Serial.println(LINE.getVersion());
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting ",  SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());
  LINE.setToken(LINE_TOKEN);
}

void loop() {
  Serial.println(digitalRead(Data));
  if(digitalRead(Data) == HIGH){
    LINE.notify("ตรวจพบการปลดล็อคตู้เซฟ\n(UNLOCKED)");
    delay(3000);
  }
}

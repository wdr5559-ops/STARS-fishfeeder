#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Servo.h>

Servo servo;

// Ganti dengan maklumat Firebase dan WiFi awak
#define FIREBASE_HOST "stars-fish-feeder-341f8-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "CKKbaqUwJIFjZGHsC7wRipqW4psFBcwrFXo0chSi"
#define WIFI_SSID "vivoY33s"
#define WIFI_PASSWORD "zaki22281"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800); // GMT+5:30 India – ubah ikut zon masa

FirebaseData timer, feed;
String stimer;
String Str[] = {"00:00", "00:00", "00:00"};
int i, feednow = 0;

void setup() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  servo.attach(D3); // Pin D3 (GPIO0)
}

void loop() {
  Firebase.getInt(feed, "/feednow");
  feednow = feed.to<int>();
  Serial.println(feednow);

  if (feednow == 1) {
    // Manual Feeding
    servo.writeMicroseconds(1000);
    delay(700);
    servo.writeMicroseconds(1500);
    feednow = 0;
    Firebase.setInt(feed, "/feednow", feednow);
    Serial.println("Fed manually");
  } else {
    // Scheduled Feeding
    for (i = 0; i < 3; i++) {
      String path = "/timers/timer" + String(i);
      Firebase.getString(timer, path);
      stimer = timer.to<String>();
      Str[i] = stimer.substring(9, 14); // ambil "HH:MM"
    }

    timeClient.update();
    String currentTime = String(timeClient.getHours()) + ":" + (timeClient.getMinutes() < 10 ? "0" : "") + String(timeClient.getMinutes());
    Serial.println("Current Time: " + currentTime);

    if (Str[0] == currentTime || Str[1] == currentTime || Str[2] == currentTime) {
      servo.writeMicroseconds(1000);
      delay(700);
      servo.writeMicroseconds(1500);
      Serial.println("Fed by schedule");
      delay(60000); // tunggu 1 minit sebelum loop semula
    }
  }

  // Reset time strings
  Str[0] = "00:00";
  Str[1] = "00:00";
  Str[2] = "00:00";
}
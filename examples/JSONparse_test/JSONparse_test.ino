//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
//Store up to 200B of data
StaticJsonDocument<200> doc;

File questionFile;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing SD Card...");
  SerialBT.begin("CareMate"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  digitalWrite(5, HIGH);
  if(!SD.begin(5)){
    Serial.println("initialization of SD card failed!");
    return;
  }
  Serial.println("SD Card Initialization Done.");
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    String tmp = SerialBT.readString();
    Serial.println(tmp);
    DeserializationError error = deserializeJson(doc, tmp);
    if(error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }
    else
    {
      const char* type = doc["type"];
      Serial.println(type);
      if(type[0] == 'n')
      {
        const String email = doc["email"];
        const char* phone = doc["phone"];
        Serial.println(email);
        Serial.println(phone);
      }
      if(type[0] == 'a')
      {
        const String day = doc["day"];
        const int time = doc["time"];
        Serial.println(day);
        Serial.println(time);
      }
      if(type[0] == 'q')
      {
        const int number = doc["number"];
        const String question = doc["question"];
        Serial.println(number);
        Serial.println(question);


        Serial.println("Reading contents of questions.txt...");
        questionFile = SD.open("questions.txt");
        if(questionFile)
        {
          while(questionFile.available())
            Serial.write(questionFile.read());

            questionFile.close();
        }
        else
          Serial.println("File failed to open!");

        Serial.print("Writing to questions.txt...");
        if(questionFile)
        {
          questionFile = SD.open("questions.txt", FILE_WRITE);
          questionFile.println(tmp);
          questionFile.close(); 
          Serial.print("Done writing to questions.txt");
        }
        else
          Serial.println("File failed to open!");
        
      }
      if(type[0] == 'm')
      {
        const int bin = doc["bin"];
        const String day = doc["day"];
        const int time = doc["time"];
        Serial.println(bin);
        Serial.println(day);
        Serial.println(time);
      }
      if(type[0] == 'w')
      {
        const String ssid = doc["SSID"];
        const String pass = doc["pass"];
        Serial.println(ssid);
        Serial.println(pass);
      }
    }
  }
  delay(20);
}
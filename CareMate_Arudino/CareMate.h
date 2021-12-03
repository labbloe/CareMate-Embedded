#ifndef CareMate_H
#define CareMate_H

// DISPLAY DEFINE
#define LITE 2
#define YP 33
#define XP 4
#define YM 0
#define XM 32
#define CARD_CS 5
#define TFT_CS 17

// INPUTS DEFINE
#define LS 22
#define BUTTON1 34
#define BUTTON2 35

// OUTPUTS DEFINE
#define SERVO 26
#define SPEAKER 25
#define MIC 27

// TOUCHSCREEN DEFINES
#define TS_MINX 115
#define TS_MINY 50
#define TS_MAXX 790
#define TS_MAXY 950
#define MINPRESSURE 10
#define MAXPRESSURE 1000

// USER DEFINES
#define SERIAL_ON 0
#define BG 0xE71C
#define BOX 0xCE59
#define TB 0x2945

// INCLUDES
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <JPEGDecoder.h>
#include "TouchScreen.h"
#include <ESP32Servo.h>
#include "BluetoothSerial.h"
#include <ArduinoJson.h>

#endif

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
#define TS_MINX 170
#define TS_MINY 105
#define TS_MAXX 780
#define TS_MAXY 940
#define MINPRESSURE 10
#define MAXPRESSURE 1000

// PRINT DEFINES
#define SERIAL_ON 1

// DISPLAY DEFINES
#define BG 0xE71C
#define BOX 0xCE59
#define TB 0x2945

#define TASK_BAR_TEXT 0
#define TIME_TEXT 1
#define MAIN_BOX_1 2
#define MAIN_BOX_2 3
#define SELECTION_BOX_1 4
#define SELECTION_BOX_2 5
#define SELECTION_BOX_3 6
#define ALARMS 7
#define MEDICATION 8
#define MESSAGES 9
#define NO_SELECTION 10

#define MAIN_SCREEN 0
#define THREE_BAR_SCREEN 1

#define WIFI_SSID "Bill Wi, the Science Fi"
#define WIFI_PASSWORD "purplepotato925"
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "care.mate.msg@gmail.com"
#define AUTHOR_PASSWORD "Adidas11"

#define RECIPIENT_EMAIL "jackschott99@gmail.com"

// INCLUDES
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <WiFi.h>
#include <Arduino.h>

#include "TFT_eSPI.h"
#include "JPEGDecoder.h"
#include "ESP32Servo.h"
#include "ArduinoJson.h"
#include "TouchScreen.h"
#include "BluetoothSerial.h"
#include "ESP_Mail_Client.h"

bool read_button(uint8_t pin);
bool dispense_pills();
bool display_text(uint8_t selection, String input);
bool display_rect(uint8_t selection);
void bluetooth_setup();
void load_second_display(const String type);
void load_second_display_array(const String type);
uint8_t check_ts(uint8_t screen_state);
void drawSdJpeg(const char *filename, int xpos, int ypos);
void jpegRender(int xpos, int ypos);
void jpegInfo();
void showTime(uint32_t msTime);

#endif

/* SAVED CODE
    -print jpeg to screen
    drawSdJpeg("/pill.jpg", 0, 0);

    -fill screen with 16 bit color
    tft.fillScreen(0xFFFF);

    -print string to screen (text, horiz, vert, size)
    tft.drawString("12:34PM", 50, 100, 7);

    -update touchscreen variable
    check_ts();
    
    -write to servo
    pillServo.write(120);
    
    -fill rectangle on screen (horiz place, vert place, horiz size, vert size, color)
    tft.fillRect(cx-i2, cy-i2, i, i, color1);
    
    -set cursor location (horiz, vert)
    tft.setCursor(0, 0);
    
    -set text color and background (text color, background color)
    tft.setTextColor(HX8357_WHITE, HX8357_BLACK);
    
    -set text size of printed text on screen
    tft.setTextSize(1);
    
    -screen layout notes
    top bar goes down 18 pixels
  */

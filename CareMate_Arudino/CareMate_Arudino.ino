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

// INCLUDES
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <JPEGDecoder.h>
#include "TouchScreen.h"
#include <ESP32Servo.h> 
#include "BluetoothSerial.h"

// OBJECT DECLARATIONS
TFT_eSPI tft = TFT_eSPI();
Servo pillServo;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint p;

// VARIABLE DECLARATIONS
uint8_t click_value = 1;
uint8_t old_click_value = 0;
uint16_t temp_x;

// BLUETOOTH CHECK
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif





// SETUP
void setup()
{
  // SERIAL
  Serial.begin(115200);

  // OUTPUTS
  pinMode(LS, OUTPUT);
  pinMode(BUTTON1, OUTPUT);
  pinMode(BUTTON2, OUTPUT);
  pinMode(SERVO, OUTPUT);
  
  // BLUETOOTH
  SerialBT.begin("CareMate"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  // SERVO
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  pillServo.setPeriodHertz(50);// Standard 50hz servo
  pillServo.attach(SERVO, 700, 2300);
  analogSetWidth(10);
  
  // SPI
  digitalWrite(TFT_CS, HIGH); // Set CS's to high to avoid bus contention
  digitalWrite(CARD_CS, HIGH);
  //digitalWrite(15, HIGH); // TFT screen chip select 
  //digitalWrite(5, HIGH); // SD card chips select, must use GPIO 5 (ESP32 SS)

  // SD CARD
  if (!SD.begin(SD_CS))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  Serial.println("initialisation done.");

  // DISPLAY
  tft.begin();
  tft.setRotation(1);  // landscape (all files printed in landscape only)
  tft.fillScreen(random(0xFFFF));
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
}





void loop()
{
  /* SAVED CODE
  drawSdJpeg("/pill.jpg", 19, 205);
  tft.fillScreen(0xFFFF);
  tft.drawString("12:34PM", 50, 100, 7); // VERT, HORIZ, SIZE
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  check_ts();
  pillServo.write(120);
  */
  
  if (Serial.available())
  {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available())
  {
    Serial.write(SerialBT.read());
  }
  delay(20);
}





void check_ts()
{
  p = ts.getPoint();

  //Serial.print("p.z:"); Serial.print(p.z); Serial.print("\tp.x:"); Serial.print(p.x); Serial.print("\tp.y:"); Serial.println(p.y);
  
  old_click_value = click_value;
  if(p.z < MINPRESSURE || p.z > MAXPRESSURE)
  {
    if(SERIAL_ON)
    {
      Serial.print("Pressure = "); Serial.print(p.z); Serial.println(" Out of pressure range!");
    }
     click_value = 0;
  }
  else
  {
    // Scale from ~0->1000 to tft.width using the calibration #'s
    temp_x = p.x;
    p.x = map(p.y, TS_MINY, TS_MINY, 0, tft.width());
    p.y = map(temp_x, TS_MINX, TS_MAXX, 0, tft.height());

    if(p.y >= 160)
    {
      if(p.x <= 160)
        click_value = 1;
      else if (p.x <= 320)
        click_value = 2;
      else
        click_value = 3;
    }
    else
    {
      click_value = 0;
    }
    
    if(SERIAL_ON)
    {
      Serial.print("X = "); Serial.print(p.x);
      Serial.print("\tY = "); Serial.print(p.y);
      Serial.print("\tPressure = "); Serial.println(p.z);  
    }
  }

  if(click_value != old_click_value)
  {
    tft.fillRect(0, 0, 480, 35, 0xFFFF);
    tft.setCursor(0,0);

    switch(click_value)
    {
      case 1:
        tft.print(" Pill pressed");
        break;
      case 2:
        tft.print(" Mail pressed");
        break;
      case 3:
        tft.print(" Alarm pressed");
        break;
      default:
        tft.print(" Nothing is clicked");
        break;
    }
  }
}

//####################################################################################################
// Draw a JPEG on the TFT pulled from SD Card
//####################################################################################################
// xpos, ypos is top left corner of plotted image
void drawSdJpeg(const char *filename, int xpos, int ypos) {

  // Open the named file (the Jpeg decoder library will close it)
  File jpegFile = SD.open( filename, FILE_READ);  // or, file handle reference for SD library
 
  if ( !jpegFile ) {
    Serial.print("ERROR: File \""); Serial.print(filename); Serial.println ("\" not found!");
    return;
  }

  Serial.println("===========================");
  Serial.print("Drawing file: "); Serial.println(filename);
  Serial.println("===========================");

  // Use one of the following methods to initialise the decoder:
  boolean decoded = JpegDec.decodeSdFile(jpegFile);  // Pass the SD file handle to the decoder,
  //boolean decoded = JpegDec.decodeSdFile(filename);  // or pass the filename (String or character array)

  if (decoded) {
    // print information about the image to the serial port
    jpegInfo();
    // render the image onto the screen at given coordinates
    jpegRender(xpos, ypos);
  }
  else {
    Serial.println("Jpeg file format not supported!");
  }
}

//####################################################################################################
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
//####################################################################################################
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void jpegRender(int xpos, int ypos) {

  //jpegInfo(); // Print information from the JPEG file (could comment this line out)

  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  bool swapBytes = tft.getSwapBytes();
  tft.setSwapBytes(true);
  
  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = min(mcu_w, max_x % mcu_w);
  uint32_t min_h = min(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // Fetch data from the file, decode and display
  while (JpegDec.read()) {    // While there is more data in the file
    pImg = JpegDec.pImage ;   // Decode a MCU (Minimum Coding Unit, typically a 8x8 or 16x16 pixel block)

    // Calculate coordinates of top left corner of current MCU
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // calculate how many pixels must be drawn
    uint32_t mcu_pixels = win_w * win_h;

    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= tft.width() && ( mcu_y + win_h ) <= tft.height())
      tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
    else if ( (mcu_y + win_h) >= tft.height())
      JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }

  tft.setSwapBytes(swapBytes);

  showTime(millis() - drawTime); // These lines are for sketch testing only
}

//####################################################################################################
// Print image information to the serial port (optional)
//####################################################################################################
// JpegDec.decodeFile(...) or JpegDec.decodeArray(...) must be called before this info is available!
void jpegInfo() {

  // Print information extracted from the JPEG file
  Serial.println("JPEG image info");
  Serial.println("===============");
  Serial.print("Width      :");
  Serial.println(JpegDec.width);
  Serial.print("Height     :");
  Serial.println(JpegDec.height);
  Serial.print("Components :");
  Serial.println(JpegDec.comps);
  Serial.print("MCU / row  :");
  Serial.println(JpegDec.MCUSPerRow);
  Serial.print("MCU / col  :");
  Serial.println(JpegDec.MCUSPerCol);
  Serial.print("Scan type  :");
  Serial.println(JpegDec.scanType);
  Serial.print("MCU width  :");
  Serial.println(JpegDec.MCUWidth);
  Serial.print("MCU height :");
  Serial.println(JpegDec.MCUHeight);
  Serial.println("===============");
  Serial.println("");
}

//####################################################################################################
// Show the execution time (optional)
//####################################################################################################
// WARNING: for UNO/AVR legacy reasons printing text to the screen with the Mega might not work for
// sketch sizes greater than ~70KBytes because 16 bit address pointers are used in some libraries.

// The Due will work fine with the HX8357_Due library.

void showTime(uint32_t msTime) {
  //tft.setCursor(0, 0);
  //tft.setTextFont(1);
  //tft.setTextSize(2);
  //tft.setTextColor(TFT_WHITE, TFT_BLACK);
  //tft.print(F(" JPEG drawn in "));
  //tft.print(msTime);
  //tft.println(F(" ms "));
  Serial.print(F(" JPEG drawn in "));
  Serial.print(msTime);
  Serial.println(F(" ms "));
}

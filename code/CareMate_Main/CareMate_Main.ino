#include "CareMate.h"

class medications
{
  public:
    int bin;
    int tim;
    String day;
};

// OBJECT DECLARATIONS
TFT_eSPI tft = TFT_eSPI();
Servo pillServo;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint p;
BluetoothSerial SerialBT;
medications meds[7];

// VARIABLE DECLARATIONS
uint8_t click_value = 1;
uint8_t old_click_value = 0;
uint8_t pills_disp = 0;
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
  digitalWrite(LS, LOW);
  pinMode(LS, INPUT_PULLUP);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
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
  //digitalWrite(TFT_CS, HIGH); // Set CS's to high to avoid bus contention
  //digitalWrite(CARD_CS, HIGH);
  digitalWrite(15, HIGH); // TFT screen chip select
  digitalWrite(5, HIGH); // SD card chips select, must use GPIO 5 (ESP32 SS)

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
  {
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
  tft.setRotation(3);  // landscape (all files printed in landscape only)
  tft.fillScreen(0x45C9);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(3);
  tft.setTextColor(TFT_BLACK, 0x45C9);
  tft.print("Test Bluetooth Mode");
  //drawSdJpeg("/main.jpg", 0, 0);
  //drawSdJpeg("/pill_alarm.jpg", 0, 0);
  bluetooth_setup();
}





void loop()
{
  drawSdJpeg("/main.jpg", 0, 0);
  display_text(TIME_TEXT, "12:55");
  display_text(TASK_BAR_TEXT, "CareMate test titble bar");
  display_text(MAIN_BOX_1, "test box 1");
  display_text(MAIN_BOX_2, "test box 2");
  delay(1000);
  display_rect(TIME_TEXT);
  display_rect(TASK_BAR_TEXT);
  display_rect(MAIN_BOX_1);
  display_rect(MAIN_BOX_2);
  delay(1000);

  drawSdJpeg("/pill_alarm.jpg", 0, 0);
  display_text(TASK_BAR_TEXT, "Test medication/alarm screen");
  display_text(SELECTION_BOX_1, "Test day 1");
  display_text(SELECTION_BOX_2, "Test day 2");
  display_text(SELECTION_BOX_3, "Test day 3");
  delay(1000);
  display_rect(TASK_BAR_TEXT);
  display_rect(SELECTION_BOX_1);
  display_rect(SELECTION_BOX_2);
  display_rect(SELECTION_BOX_3);
  delay(1000);

  //load_second_display("wifi");
  load_second_display_array("medication");
  delay(3000);
  
  

  
  /*
  if (read_button(BUTTON1))
  {

    Serial.println(dispense_pills());
  }



  if (read_button(BUTTON2))
  {
    for (uint8_t i = 0; i < 90; i++)
    {
      Serial.println(90 - i);
      pillServo.write(90 - i);
      delay(2000);
    }

  }
  pillServo.write(86);
  */
}
  


void load_second_display_array(const String type)
{
  StaticJsonDocument<1600> doc;
  File theFile;
  String SD_Read;

  theFile = SD.open(("/" + type + ".txt."));
  if (theFile)
  {
    Serial.println("Reading " + type + ".txt...");
    while (theFile.available())
    {
      SD_Read += (char)theFile.read();
    }
    theFile.close();
    Serial.println(SD_Read);

    DeserializationError error = deserializeJson(doc, SD_Read);
    if(error)
    {
      Serial.print("deserializeJson() failed: "); Serial.println(error.f_str());
    }
    else
    {
      for(uint8_t i = 0; i < 7; i++)
      {
        String value = doc["list"][i];
        StaticJsonDocument<200> tempDoc;

        DeserializationError error2 = deserializeJson(tempDoc, value);
        if(error2)
        {
          Serial.print("deserializeJson() failed: "); Serial.println(error2.f_str());
        }
        else
        {
          const int bin = tempDoc["bin"];
          const String day = tempDoc["day"];
          const int tim = tempDoc["time"];
          
          meds[i].bin = bin;
          meds[i].day = day;
          meds[i].tim = tim;

          //Serial.print("Index: "); Serial.print(i); Serial.print(" Bin: "); Serial.print(meds[i].bin); Serial.print(" Day: "); Serial.print(meds[i].bin); Serial.print(" Time: "); Serial.println(meds[i].tim);
        }
      }
      
      drawSdJpeg("/pill_alarm.jpg", 0, 0);
      display_text(TASK_BAR_TEXT, "Print test jason array");
      for(int8_t i = pills_disp; i < 7 && i - pills_disp < 3; i++)
      {
          display_text(SELECTION_BOX_1 + i, (meds[i].day + " " + meds[i].tim/100 + ":" + meds[i].tim%100));
      }
    }
  } 
  else
  {
    Serial.println("Error opening file");
  }
  
  //DeserializationError error = deserializeJson(doc, tmp);
}

void load_second_display(const String type)
{
  StaticJsonDocument<100> doc;
  File theFile;
  String SD_Read;

  theFile = SD.open(("/" + type + ".txt."));
  if (theFile)
  {
    Serial.println("Reading " + type + ".txt...");
    while (theFile.available())
    {
      SD_Read += (char)theFile.read();
    }
    theFile.close();
    Serial.println(SD_Read);

    DeserializationError error = deserializeJson(doc, SD_Read);
    if(error)
    {
      Serial.print("deserializeJson() failed: "); Serial.println(error.f_str());
    }
    else
    {
      const String ssid = doc["SSID"];  Serial.print("Wifi SSID: ");      Serial.println(ssid);
      const String pass = doc["pass"];  Serial.print("Wifi Password: ");  Serial.println(pass);
      
      drawSdJpeg("/pill_alarm.jpg", 0, 0);
      display_text(TASK_BAR_TEXT, "Test of reading Wifi data from SD card");
      display_text(SELECTION_BOX_1, ("Wifi Login"));
      display_text(SELECTION_BOX_2, (ssid));      
      display_text(SELECTION_BOX_3, (pass));    
    }
  } 
  else
  {
    Serial.println("Error opening file");
  }
  
  //DeserializationError error = deserializeJson(doc, tmp);
}

void bluetooth_setup()
{
  StaticJsonDocument<800> doc;
  File theFile;

  while(1)
  {
    if (Serial.available())
    {
      SerialBT.write(Serial.read());
    }
    if (SerialBT.available())
    {
      String tmp = SerialBT.readString();
      Serial.print("Message over BT: "); Serial.println(tmp);
      DeserializationError error = deserializeJson(doc, tmp);
      if(error)
      {
        Serial.print(F("deserializeJson() failed: ")); Serial.println(error.f_str());
      }
      else
      {
        const String type = doc["type"];
        Serial.print("Type: "); Serial.println(type);

        SD.remove(("/" + type + ".txt."));
        Serial.print("Writing to "); Serial.print(type); Serial.println(".txt...");
        theFile = SD.open(("/" + type + ".txt."), FILE_APPEND);
        if(theFile)
        {
          theFile.println(tmp);
          theFile.close(); 
          Serial.print("Done writing to "); Serial.print(type); Serial.println(".txt");
        }
        else
        {
          Serial.println("File failed to open!");
        }

        if(type == "notification")
        {
          const String number = doc["email"]; Serial.print("Notification Email: "); Serial.println(number);
          const String phone = doc["phone"];  Serial.print("Notification Phone: "); Serial.println(phone);
        }
        else if(type == "alarm")
        {
          const String day = doc["day"];  Serial.print("Alarm Day: ");  Serial.println(day);
          const String tim = doc["time"]; Serial.print("Alarm Time: "); Serial.println(tim);
        }
        else if(type == "question")
        {
          const String number = doc["number"];    Serial.print("Question Number: ");  Serial.println(number);
          const String question = doc["question"];  Serial.print("Question Text: ");  Serial.println(question);
        }
        else if(type == "medication")
        {
          const String bin = doc["bin"];  Serial.print("Medication Bin: ");   Serial.println(bin);
          const String day = doc["day"];  Serial.print("Medication Day: ");   Serial.println(day);
          const String tim = doc["time"]; Serial.print("Medication Time: ");  Serial.println(tim);
        }
        else if(type == "wifi")
        {
          const String ssid = doc["SSID"];  Serial.print("Wifi SSID: ");      Serial.println(ssid);
          const String pass = doc["pass"];  Serial.print("Wifi Password: ");  Serial.println(pass);
        }
      }
    }
    if(!digitalRead(LS))
    {
      return;
    }
  }
}

bool display_rect(uint8_t selection)
{
  switch(selection)
  {
    case TASK_BAR_TEXT:
      tft.fillRect(0, 0, 480, 18, TB);
      break;
    case TIME_TEXT:
      tft.fillRect(0, 20, 480, 90, BG);
      break;
    case MAIN_BOX_1:
      tft.fillRect(45, 116, 390, 41, BOX);
      break;
    case MAIN_BOX_2:
      tft.fillRect(45, 179, 390, 41, BOX);
      break;
    case SELECTION_BOX_1:
      tft.fillRect(46, 62, 390, 49, BOX);
      break;
    case SELECTION_BOX_2:
      tft.fillRect(46, 135, 390, 49, BOX);
      break;
    case SELECTION_BOX_3:
      tft.fillRect(46, 206, 390, 49, BOX);
      break;
    default:
      break;
  }
}

bool display_text(uint8_t selection, String input)
{
  switch(selection)
  {
    case TASK_BAR_TEXT:
      tft.setCursor(0, 0);
      tft.setTextSize(2);
      tft.setTextColor(BOX, TB);
      tft.print(input);
      break;
    case TIME_TEXT:
      tft.setTextSize(8);
      tft.setCursor(140, 40);
      tft.setTextColor(0x0000, BG);
      tft.print(input);
      break;
    case MAIN_BOX_1:
      tft.setCursor(50, 120);
      tft.setTextSize(4);
      tft.setTextColor(0x0000, BOX);
      tft.print(input);
      break;
    case MAIN_BOX_2:
      tft.setCursor(50, 182);
      tft.setTextSize(4);
      tft.setTextColor(0x0000, BOX);
      tft.print(input);
      break;
    case SELECTION_BOX_1:
      tft.setTextSize(4);
      tft.setCursor(50, 70);
      tft.setTextColor(0x0000, BOX);
      tft.print(input);
      break;
    case SELECTION_BOX_2:
      tft.setTextSize(4);
      tft.setCursor(50, 143);
      tft.setTextColor(0x0000, BOX);
      tft.print(input);
      break;
    case SELECTION_BOX_3:
      tft.setTextSize(4);
      tft.setCursor(50, 217);
      tft.setTextColor(0x0000, BOX);
      tft.print(input);
      break;
    default:
      break;
  }
}

bool dispense_pills()
{

  uint32_t my_time = millis();
  bool return_value = true;

  pillServo.write(98);
  while (!digitalRead(LS) && return_value)
  {
    if (millis() - my_time > 10000)
    {
      return_value = false;
    }
  }
  delay(5);
  while (digitalRead(LS) && return_value)
  {
    if (millis() - my_time > 10000)
    {
      return_value = false;
    }
  }
  pillServo.write(90);

  return return_value;
}

bool read_button(uint8_t pin)
{
  for (uint8_t i = 0; i < 20; i++)
  {
    if (!digitalRead(pin))
    {
      return false;
    }
    delay(1);
  }
  return true;
}

void check_ts()
{
  p = ts.getPoint();

  //Serial.print("p.z:"); Serial.print(p.z); Serial.print("\tp.x:"); Serial.print(p.x); Serial.print("\tp.y:"); Serial.println(p.y);

  old_click_value = click_value;
  if (p.z < MINPRESSURE || p.z > MAXPRESSURE)
  {
    if (SERIAL_ON)
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

    if (p.y >= 160)
    {
      if (p.x <= 160)
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

    if (SERIAL_ON)
    {
      Serial.print("X = "); Serial.print(p.x);
      Serial.print("\tY = "); Serial.print(p.y);
      Serial.print("\tPressure = "); Serial.println(p.z);
    }
  }

  if (click_value != old_click_value)
  {
    tft.fillRect(0, 0, 480, 35, 0xFFFF);
    tft.setCursor(0, 0);

    switch (click_value)
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
    //jpegInfo();
    // render the image onto the screen at given coordinates
    jpegRender(xpos, ypos);
  }
  else {
    Serial.println("Jpeg file format not supported!");
  }
  jpegFile.close();
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

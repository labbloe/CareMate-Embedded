#include "CareMate.h"

class medication_class
{
  public:
    int bin;
    int time;
    String day;
};

class notifcation_class
{
  public:
    String email;
    String phone;
};

class question_class
{
  public:
    String question;
};

class wifi_class
{
  public:
    String ssid;
    String pass;
    int time;
};

class alarm_class
{
  public:
    String day;
    int time;
};

class data_class
{
  public:
    medication_class medications[7];
    notifcation_class notification;
    question_class questions[7];
    wifi_class wifi;
    alarm_class alarms[7];
};

// OBJECT DECLARATIONS
TFT_eSPI tft = TFT_eSPI();
Servo pillServo;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint p;
BluetoothSerial SerialBT;
SMTPSession smtp;
data_class data;

// VARIABLE DECLARATIONS
uint8_t click_value = NO_SELECTION;
uint8_t last_click_value = TIME_TEXT;
uint8_t pills_disp = 0;
bool testBool = true;
uint8_t hours = 9;
uint8_t minutes = 12;
uint32_t base_time = millis();
uint8_t screen_state;
bool first_start;

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
  pinMode(LS, INPUT_PULLUP);
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
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

  drawSdJpeg("/main.jpg", 0, 0);
  screen_state = MAIN_SCREEN;
  display_text(TIME_TEXT, "BT Setup");
  display_text(TASK_BAR_TEXT, "Press top button to exit Bluetooth setup");
  bluetooth_setup();
  display_rect(TIME_TEXT);
  display_rect(TASK_BAR_TEXT);
  display_text(TASK_BAR_TEXT, "        CareMate       Team JJJJ");

  //screen_state = THREE_BAR_SCREEN;
  //drawSdJpeg("/pill_alarm.jpg", 0, 0);
  

  Serial.print("\n\nData\n");

  json_load("medication");
  json_load("alarm");
  json_load("notification");
  json_load("wifi");
  json_load("question");

  Serial.print("\n\n");

  first_start = true;
}

 


void loop()
{
  update_time();



  /*
  last_click_value = click_value;
  click_value = check_ts();

  Serial.print("Selection: ");
  Serial.println(click_value);

  if(click_value != last_click_value)
  {
    display_rect(MAIN_BOX_1);

    if(click_value == ALARMS)
    {
      Serial.println("Alarm");
      display_text(MAIN_BOX_1, "Alarms");
    }
    else if(click_value == MEDICATION)
    {
      Serial.println("Medication");
      display_text(MAIN_BOX_1, "Medication");
    }
    else if(click_value == MESSAGES)
    {
      Serial.println("Message");
      display_text(MAIN_BOX_1, "Messages");
    }
    else
    {
      Serial.println("No selection");
      display_text(MAIN_BOX_1, "No selection");
    }

    delay(100);
  }
  */
  






  /*
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
  */
  
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

void update_time()
{
  uint32_t temp_time = millis() - base_time;
  bool update_screen = false;

  if(temp_time > (1000 * 60))
  {
    update_screen = true;
    base_time += (1000 * 60);

    minutes++;
    if(minutes == 60)
    {
      minutes = 0;
      hours++;
      if(hours == 24)
      {
        hours = 0;
      }
    }
  }

  if(first_start || (screen_state == MAIN_SCREEN && update_screen))
  {
    String temp_str;
    char itoa_ca[5];

    first_start = false;
    itoa((hours == 0 ? 12 : hours % 12), itoa_ca, 10);
    temp_str += itoa_ca;
    temp_str += ":";
    itoa(minutes, itoa_ca, 10);
    temp_str += minutes < 10 ? "0" : "";
    temp_str += itoa_ca;
    temp_str += hours > 12 ? " PM" : " AM";

    Serial.println(temp_str);

    display_rect(TIME_TEXT);
    display_text(TIME_TEXT, temp_str);
  }
}

void json_load(const String type)
{
  StaticJsonDocument<1600> doc;
  File theFile;
  String SD_Read;

  theFile = SD.open(("/" + type + ".txt."));
  if (theFile)
  {
    //Serial.println("\nReading " + type + ".txt...");
    while (theFile.available())
    {
      SD_Read += (char)theFile.read();
    }
    theFile.close();
    //Serial.println(SD_Read);

    DeserializationError error = deserializeJson(doc, SD_Read);
    if(error)
    {
      Serial.print("deserializeJson() failed: "); Serial.println(error.f_str());
    }
    else
    {
      if(type == "wifi")
      {
        const String temp_ssid = doc["SSID"];
        data.wifi.ssid = temp_ssid;
        const String temp_pass = doc["pass"];
        data.wifi.pass = temp_pass;
        const int temp_time = doc["time"];
        data.wifi.time = temp_time;

        Serial.print("\n\tWifi");
        Serial.print("\n\t\tSSID\t");
        Serial.print(temp_ssid);
        Serial.print("\n\t\tpass\t");
        Serial.print(temp_pass);
        Serial.print("\n\t\ttime\t");
        Serial.print(temp_time);
      }
      else if(type == "notification")
      {
        const String temp_email = doc["email"];
        data.notification.email = temp_email;
        const String temp_phone = doc["phone"];
        data.notification.phone = temp_phone;

        Serial.print("\n\tNotification");
        Serial.print("\n\t\temail\t");
        Serial.print(temp_email);
        Serial.print("\n\t\tphone\t");
        Serial.print(temp_phone);
      }
      else
      {
        for(uint8_t i = 0; i < 7; i++)
        {
          String json_array_element = doc["list"][i];
          
          if(type == "question")
          {
            data.questions[i].question = json_array_element;

            if(!i)
            {
              Serial.print("\n\tQuestion");
            }
            Serial.print("\n\t\tques\t");
            Serial.print(json_array_element);
          }
          else
          {
            StaticJsonDocument<200> tempDoc;

            DeserializationError error2 = deserializeJson(tempDoc, json_array_element);
            if(error2)
            {
              Serial.print("deserializeJson() failed: "); Serial.println(error2.f_str());
            }
            else
            {
              if(type == "medication")
              {
                const int temp_bin = tempDoc["bin"];
                data.medications[i].bin = temp_bin;
                const String temp_day = tempDoc["day"];
                data.medications[i].day = temp_day;
                const int temp_time = tempDoc["time"];
                data.medications[i].time = temp_time;

                if(!i)
                {
                  Serial.print("\tMedication");
                }
                Serial.print("\n\t\tbin\t");
                Serial.print(temp_bin);
                Serial.print("\n\t\ttime\t");
                Serial.print(temp_day);
                Serial.print("\n\t\tday\t");
                Serial.print(temp_time);
              }
              else if(type == "alarm")
              {
                const String temp_day = tempDoc["day"];
                data.alarms[i].day = temp_day;
                const int temp_time = tempDoc["time"];
                data.alarms[i].time = temp_time;

                if(!i)
                {
                  Serial.print("\n\tAlarm");
                }
                Serial.print("\n\t\tday\t");
                Serial.print(temp_day);
                Serial.print("\n\t\ttime\t");
                Serial.print(temp_time);
              }
            }
          }
        }
      }

      /*
      drawSdJpeg("/pill_alarm.jpg", 0, 0);
      display_text(TASK_BAR_TEXT, "Print test jason array");
      for(int8_t i = pills_disp; i < 7 && i - pills_disp < 3; i++)
      {
          display_text(SELECTION_BOX_1 + i, (meds[i].day + " " + meds[i].tim/100 + ":" + meds[i].tim%100));
      }
      */
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
      tft.setCursor(90, 40);
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
  for (uint8_t i = 0; i < 10; i++)
  {
    if (!digitalRead(pin))
    {
      return false;
    }
    delay(1);
  }
  return true;
}

uint8_t check_ts()
{
  uint8_t ret_val = NO_SELECTION;
  uint16_t temp_x;

  p = ts.getPoint();

  Serial.print("p.z:"); Serial.print(p.z); Serial.print("\tp.x:"); Serial.print(p.x); Serial.print("\tp.y:"); Serial.println(p.y);

  if(p.z == -1) //(p.z < MINPRESSURE || p.z > MAXPRESSURE)
  {
    ret_val = NO_SELECTION;
  }
  else
  {
    // Scale from ~0->1000 to tft.width using the calibration #'s
    temp_x = p.x;
    p.x = map(p.y, TS_MAXY, TS_MINY, 0, tft.width());
    p.y = map(temp_x, TS_MINX, TS_MAXX, 0, tft.height());
    
    Serial.print("p.z:"); Serial.print(p.z); Serial.print("\tp.x:"); Serial.print(p.x); Serial.print("\tp.y:"); Serial.println(p.y);

    if(screen_state == MAIN_SCREEN)
    {
      if(p.y >= 240)
      {
        if(p.x <= 165)
        {
          ret_val = ALARMS;
        }
        else if(p.x >= 315)
        {
          ret_val = MESSAGES;
        }
        else
        {
          ret_val = MEDICATION;
        }
      }
    }
    else if(screen_state == THREE_BAR_SCREEN)
    {
      if(1)
      {
        Serial.print('1');
      }
    }
  }

  return ret_val;
}

//####################################################################################################
// Draw a JPEG on the TFT pulled from SD Card
//####################################################################################################
// xpos, ypos is top left corner of plotted image
void drawSdJpeg(const char *filename, int xpos, int ypos)
{
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
void jpegRender(int xpos, int ypos)
{

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
void jpegInfo()
{
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

void showTime(uint32_t msTime)
{
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
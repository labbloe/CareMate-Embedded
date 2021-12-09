#include "CareMate.h"

class int_list
{
  uint8_t num1;
  uint8_t num2;
  uint8_t num3;
};

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
    String questions[7];
    wifi_class wifi;
    alarm_class alarms[7];
    int question_time;
};

// OBJECT DECLARATIONS
TFT_eSPI tft = TFT_eSPI();
Servo pillServo;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint p;
BluetoothSerial SerialBT;
data_class data;
SMTPSession smtp;
ESP_Mail_Session session;
SMTP_Message message;

// VARIABLE DECLARATIONS
bool update_screen;
bool update_time_now;
bool responses[7];

uint8_t pills_disp = 0;
uint8_t hours = 14;
uint8_t minutes = 5;
uint8_t day = 5;
uint8_t screen_state;
uint8_t dispenses_left = 7;
uint8_t q_asked = 0;

uint32_t base_time = millis();


// BLUETOOTH CHECK
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// SETUP
void setup()
{
  delay(400);

  // SERIAL
  Serial.begin(115200);

  // OUTPUTS

  pinMode(LS, INPUT_PULLDOWN);
  pinMode(GREEN_BUTTON, INPUT_PULLDOWN);
  pinMode(RED_BUTTON, INPUT_PULLDOWN);
  pinMode(GREEN_BUTTON_VCC, OUTPUT);
  pinMode(RED_BUTTON_VCC, OUTPUT);

  pinMode(SPEAKER, OUTPUT);

  digitalWrite(SPEAKER, HIGH);
  digitalWrite(GREEN_BUTTON_VCC, HIGH);
  digitalWrite(RED_BUTTON_VCC, HIGH);

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
  
  //bluetooth_setup(); 
  

  Serial.print("\n\nData\n");
  json_load("medication");
  json_load("alarm");
  json_load("notification");
  json_load("wifi");
  json_load("question");
  Serial.print("\n\n");

  // WIFI
  char ssid[40];
  char password[40];
  data.wifi.ssid.toCharArray(ssid, 40);
  data.wifi.pass.toCharArray(password,40);

  //drawSdJpeg("/main.jpg", 0, 0);
  //display_text(TIME_TEXT, "Wifi Setup");


  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 3600 * -6;
  const int   daylightOffset_sec = 3600;

  Serial.printf("Connecting to %s ", data.wifi.ssid);
  WiFi.begin(ssid, password);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if(millis() > base_time + 15000)
    {
      break;
    }
  }
  if(millis() > base_time + 15000)
  {
    Serial.print(" NOT");
  }
  Serial.println(" CONNECTED");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  get_time_wifi();

  sendEmail_setup(smtp, session, message);
  message.addRecipient("CareMate User", data.notification.email.c_str());

  Serial.print("THING AS C_STRING"); Serial.println(data.notification.email.c_str());

  //WiFi.disconnect(true); // disconnect, no longer needed
  //WiFi.mode(WIFI_OFF);


  update_screen = true;
  screen_state = MAIN_SCREEN;
}

void loop()
{
  if(update_screen)
  {
    switch(screen_state)
    {
      case MAIN_SCREEN:
        Serial.println("MAIN_SCREEN");
        drawSdJpeg("/main.jpg", 0, 0);
        display_top_bar();
        display_text(MAIN_BOX_1, ("Dispenses Left: "));
        display_text(MAIN_BOX_2, formatted_time(data.alarms[0].day, data.alarms[0].time));
        update_screen = false;
        update_time_now = true;
        break;
      case BT_SETUP:
      Serial.println("BT_SETUP");
        drawSdJpeg("/main.jpg", 0, 0);
        display_text(TIME_TEXT, "BT Setup");
        display_text(TASK_BAR_TEXT, "  Press screen to exit Bluetooth setup");
        update_screen = false;
        bluetooth_setup();
        Serial.print("\n\nData\n");
        break;
      case ALARMS:
        Serial.println("ALARMS");
        drawSdJpeg("/pill_alarm.jpg", 0, 0);
        display_text(TASK_BAR_TEXT, "            Upcoming Alarms");
        update_screen = false;
        display_text(SELECTION_BOX_1, formatted_time(data.alarms[0].day, data.alarms[0].time));
        display_text(SELECTION_BOX_2, formatted_time(data.alarms[1].day, data.alarms[1].time));
        display_text(SELECTION_BOX_3, formatted_time(data.alarms[2].day, data.alarms[2].time));
        //trigger_alarm();
        break;
      case MEDICATION:
        Serial.println("MEDICATION");
        drawSdJpeg("/pill_alarm.jpg", 0, 0);
        display_text(TASK_BAR_TEXT, "   Upcoming Medications Dispense Times");
        update_screen = false;
        display_text(SELECTION_BOX_1, formatted_time(data.medications[0].day, data.medications[0].time));
        display_text(SELECTION_BOX_2, formatted_time(data.medications[1].day, data.medications[1].time));
        display_text(SELECTION_BOX_3, formatted_time(data.medications[2].day, data.medications[2].time));
        break;
      case MESSAGES:
        Serial.println("MESSAGES");
        drawSdJpeg("/pill_alarm.jpg", 0, 0);
        display_text(TASK_BAR_TEXT, "     Press Option to Email Message");
        update_screen = false;
        display_text(SELECTION_BOX_1, " Good Morning!");
        display_text(SELECTION_BOX_2, "    Call me");
        display_text(SELECTION_BOX_3, "     Urgent");
        break;
      case QUESTION_SCREEN:
        bool send_email = false;
        while(digitalRead(RED_BUTTON) || digitalRead(GREEN_BUTTON)) {}
        if(q_asked == 0)
        {
          display_rect(TASK_BAR_TEXT);
          display_rect(QUESTION_SCREEN);
          display_text(TASK_BAR_TEXT, "Press green for yes and red for no");
          update_screen = false;
          //drawSdJpeg("/question.jpg", 0, 0);
  
          /*
          for(uint8_t i = 0; i < 7; i++)
          {
            Serial.print("question ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(data.questions[i]);
          }*/
        }
        else if(q_asked == 7)
        {
          send_email = true;
        } 
        else if(data.questions[q_asked].length() < 3 || send_email)
        {
          String email_text = "Greetings,\n\nReults from CareMate questionnaire:\n\n";
          screen_state = MAIN_SCREEN;
          for(uint8_t i = 0; i < q_asked; i++)
          {
            Serial.print("Response ");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(responses[i]);

            if(data.questions[i].length() > 1)
            {
              email_text += data.questions[i] + '\n' + "Response: " + (responses[i] ? "Yes" : "No") + "\n\n";
            }
          }

          email_text += "Thank you for choosing the CareMate by Team JJJJ!";
          Serial.println(email_text);
          sendMail_Text(smtp, session, message, email_text);

          q_asked = 0;
          //send email
        }
        if(screen_state == QUESTION_SCREEN)
        {
          String q = " " + data.questions[q_asked];
          String temp = "";

          uint16_t index = 0;
          uint16_t str_len = q.length();

          bool null_found = false;

          display_rect(QUESTION_SCREEN);

          tft.setTextSize(4);
          tft.setTextColor(0x000, BG);
          tft.setCursor(0, 19);

          while(str_len > 1)
          {
            index = 19;
            for(uint8_t i = 0; i < 20 && !null_found; i++)
            {
              if(q[i] == '\0')
              {
                null_found = true;
                index = i;
              }
              if(q[i] == ' ')
              {
                index = i;
              }
            }

            for(uint16_t i = 0; i < index; i++)
            {
              if(q[i] != '\0')
              {
                tft.print(q[i]);
              }
            }

            
            temp = q;
            q = "";
            tft.print("\n");

            for(uint16_t i = index; i < str_len; i++)
            {
              q += temp[i];
            }

            str_len-= index;

            Serial.print("String Length: "); Serial.println(str_len);
          }

          update_screen = false;

          //display_text(QUESTION_SCREEN, data.questions[q_asked]);
          delay(500);
        }
    }
  }
  else
  {
    switch(screen_state)
    {
      case MAIN_SCREEN:
        if(digitalRead(RED_BUTTON))
        {
          minutes++;
          update_time_now = true;
          update_time();
          check_new_time();
        }
        if(digitalRead(GREEN_BUTTON))
        {
          hours++;
          update_time_now = true;
          update_time();
          delay(200);
        }
        break;
      case BT_SETUP:
        if(digitalRead(RED_BUTTON) || digitalRead(GREEN_BUTTON))
        {
          screen_state = MAIN_SCREEN;
          update_screen = true;
        }
        break;
      case QUESTION_SCREEN:
        bool red = digitalRead(RED_BUTTON);
        bool green = digitalRead(GREEN_BUTTON);

        if(red || green)
        {
          responses[q_asked] = green;
          q_asked++;
          update_screen = true;
        }
    }
  }

  if(screen_state == BT_SETUP)
  {
    bluetooth_setup();
  }

  update_time();
  check_ts();
}

String formatted_time(const String day_entry, const int time_entry)
{
  String ret_val = "";
  String temp_str;
  char itoa_ca[5];
  uint8_t temp_hours = time_entry / 100;
  uint8_t temp_mins = time_entry % 100;

  for(uint8_t i = 0; i < (day_entry == "Thursday" ? 4 : 3); i++)
  {
    ret_val += day_entry[i];
  }
  ret_val += (day_entry == "Thursday" ? " " : "  ");

  temp_hours = (temp_hours > 12 ? temp_hours % 12 : temp_hours);

  itoa((temp_hours == 0 ? 12 : temp_hours % 13), itoa_ca, 10);
  ret_val += itoa_ca;
  ret_val += ":";
  itoa(temp_mins, itoa_ca, 10);
  ret_val += temp_mins < 10 ? "0" : "";
  ret_val += itoa_ca;
  ret_val += (time_entry / 100) >= 12 ? " PM" : " AM";

  return ret_val;
}

void update_time()
{
  uint32_t temp_time = millis() - base_time;
  bool time_changed = false;
  uint8_t temp_hours;

  if(temp_time > (1000 * 60))
  {
    time_changed = true;
    base_time += (1000 * 60);
    minutes++;
    check_new_time();
  }

  if(minutes == 60)
  {
    minutes = 0;
    hours++;
    time_changed = true;
    check_new_time();
  }
  if(hours == 24)
  {
    hours = 0;
    day++;
    time_changed = true;
    check_new_time();
  }
  if(day == 7)
  {
    day = 0;
    time_changed = true;
    display_top_bar();
    check_new_time();
  }

  if(screen_state == MAIN_SCREEN && (time_changed || update_time_now))
  {
    update_time_now = false;

    String temp_str;
    char itoa_ca[5];

    temp_hours = (hours > 12 ? hours % 12 : hours);

    itoa((temp_hours == 0 ? 12 : temp_hours % 13), itoa_ca, 10);
    temp_str += itoa_ca;
    temp_str += ":";
    itoa(minutes, itoa_ca, 10);
    temp_str += minutes < 10 ? "0" : "";
    temp_str += itoa_ca;
    temp_str += hours >= 12 ? " PM" : " AM";

    Serial.println(temp_str);

    display_rect(TIME_TEXT);
    display_text(TIME_TEXT, temp_str);
    display_top_bar();
  }

  
}

void display_top_bar()
{
  String dv = "";

  dv += int_to_day(day);


  if(day == 0 || day == 1)
  {
    dv += "   ";

  }
  else if(day == 2)
  {
    dv += "  ";
  }
  else if(day == 5)
  {
    dv += "   ";
  }
  else if(day == 4 || day == 6)
  {
    dv += " ";
  }

  dv += "       CareMate       Team JJJJ";

  display_rect(TASK_BAR_TEXT);
  display_text(TASK_BAR_TEXT, dv);
}

String int_to_day(uint8_t input_day)
{
  if(input_day == 0)
  {
    return "Sunday";
  }
  else if(input_day == 1)
  {
    return "Monday";
  }
  else if(input_day == 2)
  {
    return "Tuesday";
  }
  else if(input_day == 3)
  {
    return "Wednesday";
  }
  else if(input_day == 4)
  {
    return "Thursday";
  }
  else if(input_day == 5)
  {
    return "Friday";
  }
  else
  {
    return "Saturday";
  }
}

void check_new_time()
{
  for(uint8_t i = 0; i < 7; i++)
  {
    if(data.medications[i].day == int_to_day(day) && data.medications[i].time == (hours * 100 + minutes))
    {
      dispense_pills();
    }
    if(data.alarms[i].day == int_to_day(day) && data.alarms[i].time == (hours * 100 + minutes))
    {
      trigger_alarm();
    }
    if(data.question_time == (hours * 100 + minutes))
    {
      display_questions();
    }
  }
}

void trigger_alarm()
{
  noTone(SPEAKER);
  for(uint8_t i = 0; i < 3; i++)
  {
    tone(SPEAKER, 1000, 200);
    tone(SPEAKER, 500, 100);
    tone(SPEAKER, 250, 50);
    tone(SPEAKER, 500, 100);
    tone(SPEAKER, 1000, 200);
  }
  noTone(SPEAKER);

  Serial.println("Trigger alarm");
}

void dispense_pills()
{
  dispenses_left--;
  display_rect(MAIN_BOX_1);
  display_text(MAIN_BOX_1, ("Dispenses Left: "));

  Serial.println("Dispense pills");

  pillServo.write(80);
  delay(300);
  pillServo.write(85);
  delay(1500);

  pillServo.write(90);
}

void display_questions()
{
  screen_state = QUESTION_SCREEN;
  update_screen = true;
  q_asked = 0;
  Serial.println("Display questions");
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
            data.questions[i] = json_array_element;

            if(!i)
            {
              data.question_time = doc["time"];
              Serial.print("\n\tQuestion");
              Serial.print("\n\t\ttime\t");
              Serial.print(data.question_time);
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

void get_time_wifi()
{
  struct tm timeinfo;

  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  day = timeinfo.tm_wday;
  hours = timeinfo.tm_hour;
  minutes = timeinfo.tm_min;
}

void bluetooth_setup()
{
  // BLUETOOTH
  SerialBT.begin("CareMate"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  StaticJsonDocument<800> doc;
  File theFile;
  Serial.println("We are in the function");

  while(1)
  {
    if (Serial.available())
    {
      SerialBT.write(Serial.read());
    }
    if (SerialBT.available())
    {
      String tmp = SerialBT.readString();
      Serial.print("\nMessage over BT: "); Serial.println(tmp);
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
          display_rect(MAIN_BOX_1);
          display_text(MAIN_BOX_1, (type));
          json_load(type);
        }
        else
        {
          Serial.println("File failed to open!");
        }
      }
    }
    /*
    if(digitalRead(LS))
    {
      return;
    }
    */
    check_ts();
    if(screen_state != BT_SETUP)
    {
      // BLUETOOTH
      SerialBT.flush();  
      SerialBT.disconnect();
      SerialBT.end();
      Serial.println("Bluetooth has been disconnected");
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
    case QUESTION_SCREEN:
      tft.fillRect(0, 19, 480, 301, BG);
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
      tft.setCursor(46, 126);
      tft.setTextSize(3);
      tft.setTextColor(0x0000, BOX);
      tft.print(input);
      if(input == "Dispenses Left: ")
      {
        tft.print(dispenses_left);
      }
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
    case QUESTION_SCREEN:
      tft.setTextSize(4);
      tft.setCursor(0, 19);
      tft.setTextColor(0x000, BG);
      tft.print(input);
    default:
      break;
  }
}

uint8_t read_button(uint8_t pin)
{
  uint8_t true_track = 0;
  uint8_t false_track = 0;

  for (uint8_t i = 0; i < 10; i++)
  {
    if(digitalRead(pin))
    {
      true_track++;
    }
    else
    {
      false_track++;
    }
    delay(1);
  }

  if(true_track == 10)
  {
    return 1;
  }
  else if(false_track == 10)
  {
    return 0;
  }
  else
  {
    return 2;
  }
}

uint8_t check_ts()
{
  uint16_t temp_x;

  p = ts.getPoint();

  //Serial.print("p.z:"); Serial.print(p.z); Serial.print("\tp.x:"); Serial.print(p.x); Serial.print("\tp.y:"); Serial.println(p.y);

  if(p.z > MINPRESSURE) //(p.z < MINPRESSURE || p.z > MAXPRESSURE)
  {
    // Scale from ~0->1000 to tft.width using the calibration #'s
    temp_x = p.x;
    p.x = map(p.y, TS_MAXY, TS_MINY, 0, tft.width());
    p.y = map(temp_x, TS_MINX, TS_MAXX, 0, tft.height());
    
    //Serial.print("p.z:"); Serial.print(p.z); Serial.print("\tp.x:"); Serial.print(p.x); Serial.print("\tp.y:"); Serial.println(p.y);

    if(screen_state == MAIN_SCREEN)
    {
      if(p.y >= 240)
      {
        if(p.x <= 165)
        {
          screen_state = ALARMS;
          update_screen = true;
        }
        else if(p.x >= 315)
        {
          screen_state = MESSAGES;
          update_screen = true;
        }
        else
        {
          screen_state = MEDICATION;
          update_screen = true;
        }
      }
      else if(p.y <= 40 && p.x <= 40)
      {
        screen_state = BT_SETUP;
        update_screen = true;
      }
    }
    else if(screen_state == ALARMS)
    {
      if(p.y >= 260 && p.x <= 170)
      {
        screen_state = MAIN_SCREEN;
        update_screen = true;
      }
    }
    else if(screen_state == MESSAGES)
    {
      if(p.y >= 250 && p.x <= 170)
      {
        screen_state = MAIN_SCREEN;
        update_screen = true;
      }
      else if(p.x >= 40 && p.x <= 400 && p.y >= 55 && p.y <= 115)
      {
        // Good Morning
        display_rect(SELECTION_BOX_1);
        display_text(SELECTION_BOX_1, "  Sending...");
        String call_me_str = "Greetings,\n\nYour CareMate commands you to have a good morning!";
        sendMail_Text(smtp, session, message, call_me_str);
        delay(1000);
        display_rect(SELECTION_BOX_1);
        display_text(SELECTION_BOX_1, " Good Morning!");
      }
      else if(p.x >= 40 && p.x <= 400 && p.y >= 130 && p.y <= 188)
      {
        // Call me
        display_rect(SELECTION_BOX_2);
        display_text(SELECTION_BOX_2, "  Sending...");
        String call_me_str = "Greetings,\n\nYour CareMate indicates a request for a call.";
        sendMail_Text(smtp, session, message, call_me_str);
        delay(1000);
        display_rect(SELECTION_BOX_2);
        display_text(SELECTION_BOX_2, "    Call me");
      }
      else if(p.x >= 40 && p.x <= 400 && p.y >= 200 && p.y <= 240)
      {
        // Urgent
        display_rect(SELECTION_BOX_3);
        display_text(SELECTION_BOX_3, "  Sending...");
        String urgent_str = "Attention,\n\nURGENT MESSAGE FROM CAREMATE";
        sendMail_Text(smtp, session, message, urgent_str);
        delay(1000);
        display_rect(SELECTION_BOX_3);
        display_text(SELECTION_BOX_3, "     Urgent");
      }
    }
    else if(screen_state == MEDICATION)
    {
      if(p.y >= 260 && p.x <= 170)
      {
        screen_state = MAIN_SCREEN;
        update_screen = true;
      }
    }
    else if(screen_state == BT_SETUP)
    {
      screen_state = MAIN_SCREEN;
      update_screen = true;
    }
  }
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
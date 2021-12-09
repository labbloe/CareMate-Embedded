/*
    CareMate Send Email Header
    Jared Allen & Jack Schott
*/

#ifndef SEND_EMAIL_H
#define SEND_EMAIL_H


#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>

//Assume WiFi.h has SSID and PASS already configured

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465   //not available for Outlook

//Log-in credentials
#define AUTHOR_EMAIL "care.mate.msg@gmail.com"
#define AUTHOR_PASSWORD "Adidas11"

//Add following to setup script
/*
//SMTP session object used for email sending
SMTPSession smtp;

//Session Config Data
ESP_Mail_Session session;

//Declare message class
SMTP_Message message;

*/

void smtpCallback(SMTPSession &smtp, SMTP_Status status);

void sendEmail_setup(SMTPSession &smtp, ESP_Mail_Session &session, SMTP_Message &message);

void sendMail_Text(SMTPSession &smtp, ESP_Mail_Session &session, SMTP_Message &message, String textMessage);

void sendMail_wmv(SMTPSession &smtp, ESP_Mail_Session &session, SMTP_Message &message, String textMessage, const char* filename, const char* filepath);

#endif /* SEND_EMAIL_H */
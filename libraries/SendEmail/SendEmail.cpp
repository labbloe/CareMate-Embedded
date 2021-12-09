/*
    SendEmail Functions
*/

#include <SendEmail.h>

void sendEmail_setup(SMTPSession &smtp, ESP_Mail_Session &session, SMTP_Message &message)
{
    //Enable debug via serial port
    smtp.debug(1);

    //Set the callback function to get the sending results
    //smtp.callback(smtpCallback);

    //Set session config data
    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;
    session.login.email = AUTHOR_EMAIL;
    session.login.password = AUTHOR_PASSWORD;
    session.login.user_domain = "";

    /* Set the message headers */
    message.sender.name = "CareMate";
    message.sender.email = AUTHOR_EMAIL;
    message.subject = "Update from your CareMate";
    
}

void sendMail_Text(SMTPSession &smtp, ESP_Mail_Session &session, SMTP_Message &message, String textMessage)
{
    //use message.addRecipient(name, email) before sending mail
    message.text.content = textMessage.c_str();
    message.text.charSet = "us-ascii";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
    message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

    //Connect to server using config
    if(!smtp.connect(&session))
    {
        Serial.println("Error connecting to SMTP session!");
        return;
    }

    if(!MailClient.sendMail(&smtp, &message))
        Serial.println("Error sending Email, " + smtp.errorReason());
}

/*
    SMTP Callback should be put in loop until status == success
    or until a timeout is reached.
*/
void smtpCallback(SMTPSession &smtp, SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}

void sendMail_wmv(SMTPSession &smtp, ESP_Mail_Session &session, SMTP_Message &message, String textMessage, const char* filename, const char* filepath)
{

    //use message.addRecipient(name, email) before sending mail
    message.text.content = textMessage.c_str();
    message.text.charSet = "us-ascii";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
    message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

    SMTP_Attachment att;
    att.descr.filename = filename;
    att.descr.mime = "audio/wav";
    att.file.path = filepath;
    att.file.storage_type = esp_mail_file_storage_type_sd;
    att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;

    message.addAttachment(att);

    //Connect to server using config
    if(!smtp.connect(&session))
    {
        Serial.println("Error connecting to SMTP session!");
        return;
    }
    /* Start sending the Email and close the session */
    if (!MailClient.sendMail(&smtp, &message, true))
        Serial.println("Error sending Email, " + smtp.errorReason());
}
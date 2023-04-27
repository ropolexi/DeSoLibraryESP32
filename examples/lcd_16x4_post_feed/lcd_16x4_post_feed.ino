#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 16, 4);

// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";
const char username[] = "ropolexi";
DeSoLib deso;
DeSoLib::Profile profile1;
int server_index = 0;
void printLineLCD(int len, char *body);

void nextServer()
{
  server_index++; // try different nodes
  if (server_index >= deso.getMaxNodes())
    server_index = 0;
}

void setup()
{
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("DeSo Post Feed");
  lcd.setCursor(0, 1);
  lcd.print("(Bitclout)");
  Serial.begin(9600);
  Serial.setDebugOutput(true);
  WiFi.enableSTA(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecting");
  WiFi.begin(ssid, wifi_pass);
  while (!WiFi.isConnected())
  {
    delay(1000);
    Serial.print(".");
  }
  deso.addNodePath("https://desocialworld.com", "");
  deso.addNodePath("https://diamondapp.com", "");
  deso.addNodePath("https://node.deso.org", "");

  deso.selectDefaultNode(0);
  int status = false;
  while (!status)
  {
    status = deso.updateSingleProfile(username, "", &profile1);
    if (!status)
    {
      Serial.println("Single profile error!");
      nextServer();
    }
  }
  Serial.printf("\nUser:%s\n", profile1.Username);
  lcd.setCursor(0, 0);
  lcd.print("               ");
  lcd.setCursor(0, 0);
  lcd.print("Node:");
  printLineLCD(strlen(deso.getSelectedNodeUrl()), deso.getSelectedNodeUrl());
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Downloading");
  lcd.setCursor(0, 1);
  lcd.print("following feeds");
  lcd.setCursor(-4, 2);
  lcd.print("for ");
  lcd.setCursor(-4, 3);
  lcd.print(username);
}

void bodyCLearLCD()
{
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0 - 4, 2);
  lcd.print("                ");
  lcd.setCursor(0 - 4, 3);
  lcd.print("                ");
}
void filter(char *body, char filter_ch)
{
  char *ret;
  do
  {
    ret = strchr(body, filter_ch);
    if (ret == NULL)
      break;
    while (*ret != ' ')
    {
      *ret = 24;
      ret++;
      if (*ret == '\0')
        break;
    }
  } while (ret != NULL);
}

void filter_links(char *body)
{
  char *ret;
  ret = strstr(body, "http");
  if (ret != NULL)
  {
    Serial.println("Found link");

    while (*ret != ' ')
    {
      *ret = 24;
      ret++;
      if (*ret == '\0')
        break;
    }
  }
}
void printLineLCD(int len, char *body)
{
  int index = 0;
  int row = 1;
  int col = 0;
  lcd.setCursor(col, row);

  while (len > 0)
  {

    if (body[index] >= 32 && body[index] <= 126)
    {
      lcd.write(body[index]);
      col++;
      delay(50);
    }
    index++;

    len--;
    if (col >= 16)
    {
      row++;
      if (row > 3)
      {
        row = 1;
        delay(2000);
        bodyCLearLCD();
      }
      col = 0;
      if (row == 2 || row == 3)
      {
        lcd.setCursor(col - 4, row);
      }
      else
      {
        lcd.setCursor(col, row);
      }
    }
  }
}

void updatePostFeedLCD(const char *username, char *body)
{
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(username);
  bodyCLearLCD();

  delay(100);
  int len = strlen(body);
  printLineLCD(len, body);
}
void loop()
{
  static unsigned long timer = -60000UL;
  while (true)
  {
    if (WiFi.isConnected())
    {
      if (millis() - timer > 60000UL)
      {
        timer = millis();
        Serial.println("=== Posts Feed ===");
        int status = deso.updatePostsStatelessSave("", profile1.PublicKeyBase58Check, true, 10, false, 1);
        if (status == 0)
        {
          nextServer();
        }
        Serial.println("\n=== Posts End ===");
        while (millis() - timer < 60000UL)
        {
          for (int i = 0; i < deso.feeds.size(); i++)
          {
            Serial.printf("[%s]%s\n", deso.feeds[i].username, deso.feeds[i].body);
            filter(deso.feeds[i].body, '#');
            filter(deso.feeds[i].body, '@');
            filter_links(deso.feeds[i].body);
            updatePostFeedLCD(deso.feeds[i].username, deso.feeds[i].body);
            delay(2000);
          }
        }
      }
    }
  }
}

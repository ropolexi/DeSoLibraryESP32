#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "customchars.h"

// device :esp32-cam
// #define I2C_SDA 14 // SDA Connected to GPIO 14
// #define I2C_SCL 15 // SCL Connected to GPIO 15

// Set the LCD address to 0x27 for a 16 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 16, 4);

// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";
const char *username = "ropolexi";

DeSoLib deso;
DeSoLib::Profile profile1;
int server_index = 0;

void nextServer()
{
  server_index++; // try different nodes
  if (server_index >= deso.getMaxNodes())
    server_index = 0;
}
void skipNotWorkingNode()
{
  do
  {
    deso.selectDefaultNode(server_index);
    deso.updateNodeHealthCheck();
    if (!deso.getSelectedNodeStatus())
    {
      nextServer();
    }
  } while (!deso.getSelectedNodeStatus());
  Serial.print("\nDeSo Node: ");
  Serial.println(deso.getSelectedNodeUrl());
}

void setup()
{
  // Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, deso_logo); // custom char for deso logo
  lcd.createChar(1, diamond);   // custom char for diomand icon
  lcd.createChar(2, heart);     // custom char for like icon
  lcd.clear();
  lcd.print("DeSo Dashbaord");
  lcd.setCursor(0, 1);
  lcd.print("(Bitclout)");
  Serial.begin(9600);
  Serial.setDebugOutput(true);
  WiFi.enableSTA(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, wifi_pass);
  while (!WiFi.isConnected())
  {
    delay(1000);
    Serial.print(".");
  }
  deso.addNodePath("https://desocialworld.com", ISRG_Root_caRootCert);
  deso.addNodePath("https://diamondapp.com", Baltimore_Root_caRootCert);
  deso.addNodePath("https://node.deso.org", GTS_Root_caRootCert);

  deso.selectDefaultNode(0);
  lcd.setCursor(-4, 2);
  lcd.print("Updating");
}
void updateDisplay()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  char str_url[17];
  // strncpy(str_url, &deso.getSelectedNodeUrl()[0] + 8, 16); //limit to 16 charactors
  lcd.print(username);

  lcd.setCursor(0, 1);
  lcd.print("D:$");
  double temp = deso.USDCentsPerBitCloutExchangeRate / 100.0;
  lcd.print(temp, 0);

  double coinPriceUSDCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.CoinPriceBitCloutNanos / 1000000000.0);
  temp = coinPriceUSDCents / 100.0;
  lcd.setCursor(0 - 4, 2);
  lcd.print("        ");
  lcd.setCursor(0 - 4, 2);
  lcd.print("C:$");
  lcd.print(temp, 1);

  double balanceCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.BalanceNanos / 1000000000.0);
  temp = balanceCents / 100.0;
  lcd.setCursor(8 - 4, 2);
  lcd.print("        ");
  lcd.setCursor(8 - 4, 2);
  lcd.print("B:$");
  lcd.print(temp, 1);

  double assetsValue = (profile1.TotalHODLBalanceClout * deso.USDCentsPerBitCloutExchangeRate) / 100.0;
  lcd.setCursor(8, 1);
  temp = assetsValue;
  lcd.print("H:$");
  lcd.print(temp, 1);

  lcd.setCursor(-4, 3);
  lcd.write(2);
  lcd.print(" ");
  lcd.print(profile1.lastNPostLikes);

  lcd.setCursor(-4 + 8, 3);
  lcd.write(1);
  lcd.print(" ");
  lcd.print(profile1.lastNPostDiamonds);
}
void updateHoldersDisplay()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("*CC Top Holders*");
  lcd.setCursor(0, 1);
  lcd.print(profile1.TopHodlersUserNames[1]);
  lcd.setCursor(-4, 2);
  lcd.print(profile1.TopHodlersUserNames[2]);
  lcd.setCursor(-4, 3);
  lcd.print(profile1.TopHodlersUserNames[3]);
}
void loop()
{
  static unsigned long timer = -60000UL;
  static bool initial_run = true;
  while (true)
  {
    if (WiFi.isConnected())
    {
      if (millis() - timer > 60000UL)
      {
        timer = millis();

        skipNotWorkingNode();
        if (initial_run)
        {
          lcd.setCursor(-4, 3);
          lcd.print("                ");
          lcd.setCursor(-4, 3);
          lcd.print("Exchange Rates..");
        }
        if (!deso.updateExchangeRates())
        {
          Serial.println("exchange error!");
          nextServer();
          continue;
        }
        if (initial_run)
        {
          lcd.setCursor(-4, 3);
          lcd.print("                ");
          lcd.setCursor(-4, 3);
          lcd.print("Profile..");
        }
        int status = deso.updateSingleProfile(username, "", &profile1);
        if (!status)
        {
          Serial.println("single profile error!");
          nextServer();
          continue;
        }

        Serial.println("Updating balance");
        if (initial_run)
        {
          lcd.setCursor(-4, 3);
          lcd.print("                ");
          lcd.setCursor(-4, 3);
          lcd.print("Balance..");
        }
        deso.updateUsersBalance(profile1.PublicKeyBase58Check, &profile1);
        Serial.println("Updating hodling asset balance");
        if (initial_run)
        {
          lcd.setCursor(-4, 3);
          lcd.print("                ");
          lcd.setCursor(-4, 3);
          lcd.print("Hodling Assets..");
        }
        deso.updateHodleAssetBalance("", profile1.PublicKeyBase58Check, &profile1);
        Serial.println("Updating last 10 post likes and diamond count");
        if (initial_run)
        {
          lcd.setCursor(-4, 3);
          lcd.print("                ");
          lcd.setCursor(-4, 3);
          lcd.print("Last 10 posts..");
        }
        deso.updateLastNumPostsForPublicKey(profile1.PublicKeyBase58Check, 10, &profile1);
        Serial.println("Updating top holders");
        if (initial_run)
        {
          lcd.setCursor(-4, 3);
          lcd.print("                ");
          lcd.setCursor(-4, 3);
          lcd.print("Top holders..");
        }
        deso.updateTopHolders("", profile1.PublicKeyBase58Check, 4, &profile1);
       
        do
        {
          updateDisplay();
          delay(5000);
          updateHoldersDisplay();
          delay(5000);
        }while(millis() - timer < 60000UL);

        initial_run = false;
      }
      nextServer();
    }
  }
}
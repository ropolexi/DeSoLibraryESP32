#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";

const char userPublicKey[] = "BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8";
DeSoLib deso;
int server_index = 0;

void setup()
{
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("DeSo Dashboard");
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
  deso.addNodePath("https://desocialworld.com", GTS_Root_R1_caRootCert);
  deso.addNodePath("https://diamondapp.com", Baltimore_Root_caRootCert);
  deso.addNodePath("https://node.deso.org", GTS_Root_R1_caRootCert);

  deso.selectDefaultNode(0);
  lcd.clear();
}
void nextServer()
{
  server_index++; // try different nodes
  if (server_index >= deso.getMaxNodes())
    server_index = 0;
}

void updateBalanceLCD(double value)
{
  lcd.setCursor(8, 1);
  lcd.print("        ");
  lcd.setCursor(8, 1);
  lcd.print("B:$");
  lcd.print(value, 1);
}
void updateHodingsLCD(double value)
{
  lcd.setCursor(8, 0);
  lcd.print("        ");
  lcd.setCursor(8, 0);
  lcd.print("H:$");
  lcd.print(value, 1);
}
void updateDeSoPriceLCD(double value)
{
  lcd.setCursor(0, 0);
  lcd.print("        ");
  lcd.setCursor(0, 0);
  lcd.print("D:$");
  lcd.print(value, 1);
}
void updateCreatorCoinLCD(double value)
{
  lcd.setCursor(0, 1);
  lcd.print("        ");
  lcd.setCursor(0, 1);
  lcd.print("C:$");
  lcd.print(value, 1);
}
void loop()
{
  while (true)
  {
    if (WiFi.isConnected())
    {
      do
      {
        deso.selectDefaultNode(server_index);
        if (!deso.updateNodeHealthCheck())
        {
          nextServer();
        }
      } while (!deso.getSelectedNodeStatus());

      Serial.println();
      Serial.print("DeSo Node: ");
      Serial.println(deso.getSelectedNodeUrl());

      if (!deso.updateExchangeRates())
      {
        Serial.println("exchange error!");
        nextServer();
        continue;
      }
      Serial.print("DeSo Coin Value: $");
      double temp = deso.USDCentsPerBitCloutExchangeRate / 100.0;
      Serial.println(temp);

      updateDeSoPriceLCD(temp);

      // Serial.println("BTC (USD):");
      // Serial.println(deso.USDCentsPerBitcoinExchangeRate/100.0);
      Serial.println("=======Profile========");
      DeSoLib::Profile profile1;
      delay(1000);
      // deso.updateSingleProfile("ropolexi", "" ,&profile1);//search by username or public key
      int status = deso.updateSingleProfile("", userPublicKey, &profile1);
      if (!status)
      {
        Serial.println("single profile error!");
        nextServer();
        continue;
      }
      Serial.print("Username: ");
      Serial.println(profile1.Username);
      Serial.print("PublicKey: ");
      Serial.println(profile1.PublicKeyBase58Check);
      Serial.print("Creator Coin Price: $");

      double coinPriceUSDCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.CoinPriceBitCloutNanos / 1000000000.0);
      temp = coinPriceUSDCents / 100.0;
      Serial.println(temp);

      updateCreatorCoinLCD(temp);


      Serial.print("Wallet Balance:");
      double balanceCents = deso.USDCentsPerBitCloutExchangeRate * ((profile1.UnconfirmedBalanceNanos + profile1.BalanceNanos) / 1000000000.0);

      temp = balanceCents / 100.0;
      Serial.print(" $");
      Serial.println(temp);

      updateBalanceLCD(temp);
      

      if (deso.updateHodleAssetBalance("", profile1.PublicKeyBase58Check, &profile1))
      {
        Serial.print("Total HODLE assets : ");
        Serial.println(profile1.TotalHodleNum);
        Serial.print("Total HODLE Asset Balance: $");
        temp = (profile1.TotalHODLBalanceClout * deso.USDCentsPerBitCloutExchangeRate) / 100.0;
        Serial.println(temp);

        updateHodingsLCD(temp);
      }
      Serial.println("======================");
    }
    delay(10000UL);
    nextServer();
  }
}

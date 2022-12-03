/*
This code only check the balance difference , if the device is restarted 
it will reset and wait for another balance differnce.

Here the balance is checked with the unconfirmed balance to make it fast,
if you are doing large sums better remove unconfirmed balance to make sure
it is confirmed.
*/
#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
#include <Wire.h>

#include <LiquidCrystal_I2C.h>  //https://github.com/johnrickman/LiquidCrystal_I2C

// Set the LCD address to 0x27 for a 16 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 16, 4);

//Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";

//public key of the vending machine
const char publicKey[] = "";

//price of the item
double price = 0.001; //in dolers

int server_index;
DeSoLib deso;
DeSoLib::Profile profile1;

double preBalanceUSD = -1;
double balanceUSD = 0;

void esp_init()
{
  pinMode(GPIO_NUM_2, OUTPUT);
  digitalWrite(GPIO_NUM_2, HIGH);

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  delay(100);
  lcd.print("Vending Machine");
  lcd.setCursor(0, 1);
  lcd.print("Powered by DeSo");

  Serial.begin(9600);
  Serial.setDebugOutput(true);
  WiFi.enableSTA(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, wifi_pass);
  lcd.setCursor(0-4, 2);
  lcd.print("WiFi...");
  while (!WiFi.isConnected())
  {
    delay(1000);
    Serial.print(".");
  }
  lcd.setCursor(0-4, 2);
  lcd.print("WiFi OK");
}

void skipNotWorkingNode()
{
  do
  {
    deso.selectDefaultNode(server_index);
    deso.updateNodeHealthCheck();
    if (!deso.getSelectedNodeStatus())
    {
      server_index++;
      if (server_index >= deso.getMaxNodes())
        server_index = 0;
    }
  } while (!deso.getSelectedNodeStatus());
  Serial.print("\nDeSo Node: ");
  Serial.println(deso.getSelectedNodeUrl());
}

void setup()
{
  // put your setup code here, to run once:
  esp_init();
  deso.addNodePath("https://bitclout.com", bitclout_caRootCert);
  deso.addNodePath("https://nachoaverage.com", nachoaverage_caRootCert);
  deso.addNodePath("https://members.giftclout.com", giftclout_caRootCert);
  deso.selectDefaultNode(0);
  lcd.setCursor(0-4, 2);
  lcd.print("Update Exchage");
  deso.updateExchangeRates();
  lcd.setCursor(0-4, 2);
  lcd.print("Exchange OK   ");
}
void convertDesoUSD()
{
  balanceUSD = (((profile1.BalanceNanos + profile1.UnconfirmedBalanceNanos) / 1000000000.0) * deso.USDCentsPerBitCloutExchangeRate) / 100.0;
}
void updateDisplay()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pay: $");
  lcd.print(price, 4);
  lcd.setCursor(0 - 4, 2);
  lcd.print("=> ");
  lcd.write(' $');
  
  double remaining = balanceUSD - preBalanceUSD;
  lcd.print(remaining, 4);
  Serial.print ("\n=> $");
  Serial.println(remaining,4);
  if (remaining < price)
  {
    lcd.setCursor(0 - 4, 3);
    lcd.print("Insufficient");
    Serial.println("Insufficient");
  }
  else
  {
    lcd.setCursor(0, 1);
    Serial.println("Payment OK");
  }
}
void loop()
{
  if (WiFi.isConnected())
  {
    skipNotWorkingNode();
    Serial.println("Refreshing balance.");
    deso.updateUsersBalance(publicKey, &profile1);

    convertDesoUSD();
    if (balanceUSD-preBalanceUSD>0.0001)
    {
      deso.updateExchangeRates();
      convertDesoUSD();
      if (preBalanceUSD < 0 && balanceUSD > 0) //at start of the loop
      {
        preBalanceUSD = balanceUSD;
        updateDisplay();
      }
      else
      {
        updateDisplay();
        delay(100);
        double diff = balanceUSD - preBalanceUSD;
        while (diff >= price)
        {
          preBalanceUSD += price;
          lcd.setCursor(0 - 4, 3);
          lcd.print("Take Coffe");
          Serial.println("Output===>");
          digitalWrite(GPIO_NUM_2, LOW);
          delay(3000);
          digitalWrite(GPIO_NUM_2, HIGH);
          delay(1000);
          updateDisplay();
          diff = balanceUSD - preBalanceUSD;
        }
        lcd.setCursor(0 - 4, 3);
        lcd.print("Have a nice day");
        Serial.println("Have a nice day");
        delay(5000);
        updateDisplay();
      }
    }
    delay(5000);
    server_index++; //select a different node
    if (server_index >= deso.getMaxNodes())
      server_index = 0;
  }
}
#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
DeSoLib deso;

//fill in wifi details
const char ssid[] = "";
const char wifi_pass[] = "";

DeSoLib::Profile profile1; //variable to store user profile details

//initialize wifi and serial port
void esp_init()
{
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
  Serial.println("Connected to Wifi.");
}

void setup()
{
  esp_init();
  deso.addNodePath("https://bitclout.com", bitclout_caRootCert);
  deso.selectDefaultNode(0);
}

void loop()
{
  if (WiFi.isConnected())
  {
    Serial.println("Refreshing rates..");
    deso.updateExchangeRates();

    Serial.print("DeSo Coin Value: $");
    Serial.println(deso.USDCentsPerBitCloutExchangeRate / 100.0);
    Serial.print("BTC: $");
    Serial.println(deso.USDCentsPerBitcoinExchangeRate / 100.0);
  }
  delay(5000); //delay 5 seconds
}
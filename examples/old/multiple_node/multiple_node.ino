#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"

//Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";

DeSoLib deso;
int server_index = 0;

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
  esp_init();
  deso.addNodePath("https://bitclout.com", bitclout_caRootCert);
  deso.addNodePath("https://nachoaverage.com", nachoaverage_caRootCert);
  deso.addNodePath("https://members.giftclout.com", giftclout_caRootCert);
}

void loop()
{
  if (WiFi.isConnected())
  {
    skipNotWorkingNode();//skip not working node untill working node found
    Serial.println("Refreshing rates..");
    deso.updateExchangeRates();

    Serial.print("DeSo Coin Value: $");
    Serial.println(deso.USDCentsPerBitCloutExchangeRate / 100.0);
    Serial.print("BTC: $");
    Serial.println(deso.USDCentsPerBitcoinExchangeRate / 100.0);
  }

  delay(1000); //delay 1 second
  server_index++; //try different nodes
  if (server_index >= deso.getMaxNodes())
    server_index = 0;
}
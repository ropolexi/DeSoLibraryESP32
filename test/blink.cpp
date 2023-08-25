#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
#define OUTPUT1_PIN 15
#define LED_PIN 2
// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";
const char postHash[] = "12f33777a5911c6c1229982057b2c03ba31a3bc031056bb2d3119714ddb0ac4f";
int serialNumber = 1;

DeSoLib deso;
int server_index = 0;
DeSoLib::Profile profile1;
DeSoLib::Post post1;
void setup()
{
  // put your setup code here, to run once:
  pinMode(OUTPUT1_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(OUTPUT1_PIN, HIGH);
  digitalWrite(LED_PIN, LOW);
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
}
void nextServer()
{
  server_index++; // try different nodes
  if (server_index >= deso.getMaxNodes())
    server_index = 0;
}

void loop()
{

  static unsigned long timer1=0;
  static unsigned long timer2=0;

  while (true)
  {
    if (WiFi.isConnected())
    {
      if(millis()-timer2>200){
        timer2=millis();
        digitalWrite(LED_PIN,!digitalRead(LED_PIN));
      }
      if (millis() - timer1 > 10000)
      {
        digitalWrite(LED_PIN,HIGH);
        timer1 = millis();
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

        char owner[56];
        if (deso.getNFTEntriesForNFTPost(postHash, serialNumber, owner))
        {
          Serial.printf("NFT post hash:%s\n", postHash);
          Serial.printf("Serial Number:%d\n", serialNumber);
          Serial.printf("Owner:%s\n", owner);
          int count;
          if (deso.countPostAssociationSingle(owner, postHash, "LIKE", &count))
          {
            Serial.printf("Like:%d\n", count);
            if (count == 1)
            {
              digitalWrite(OUTPUT1_PIN, LOW);
            }
            else
            {
              digitalWrite(OUTPUT1_PIN, HIGH);
            }
          }
          else
          {
            Serial.println("Error");
          }
          if(deso.updateUsersBalance(owner, &profile1)){
            
          }
        }
        nextServer();
      }
    }
  }
}

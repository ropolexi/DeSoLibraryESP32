#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
#define LED_PIN 15
// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";
const char postHash[] = "3b49bdcf680d339c533a76c88789fa894ab49deb1628b4d87de4266cb4209462";
int serialNumber = 1;

DeSoLib deso;
int server_index = 0;
DeSoLib::Profile profile1;
DeSoLib::Post post1;
void setup()
{
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
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

      char owner[56];
      if (deso.getNFTEntriesForNFTPost(postHash, serialNumber, owner))
      {
        Serial.printf("NFT post hash:%s\n",postHash);
        Serial.printf("Serial Number:%d\n",serialNumber);
        Serial.printf("Owner:%s\n",owner);
        int count;
        if (deso.countPostAssociationSingle(owner, postHash, "LIKE", &count))
        {
          Serial.printf("Like:%d\n", count);
          if (count == 1)
          {
            digitalWrite(LED_PIN, HIGH);
          }
          else
          {
            digitalWrite(LED_PIN, LOW);
          }
        }
        else
        {
          Serial.println("Error");
        }
      }
    }
    delay(60000UL);
    nextServer();
  }
}

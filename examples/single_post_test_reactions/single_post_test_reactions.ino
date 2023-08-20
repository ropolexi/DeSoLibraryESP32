#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
#define LED_PIN 15
// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";

const char userPublicKey[] = "BC1YLhBH8oPqRRAejbUu9BbFquhLFz6GQ8ZbKk6SAWtFsbdcthVTEw8";
const char postHash[]="a6039c9a223d5fad11b05d54a152d0f77048a1552e06e7430697848db60abf71";

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
      DeSoLib::ReactionCount reactionCount;
      if(deso.countPostAssociation(userPublicKey, postHash, &reactionCount)){
        Serial.printf("\nTotal Reactions:%d\n",reactionCount.total);
        Serial.printf("Like:%d\n",reactionCount.like);
        if(reactionCount.like ==1){
          digitalWrite(LED_PIN,HIGH);
        }else{
          digitalWrite(LED_PIN,LOW);
        }
      }else{
        Serial.println("Error");
      }
      
    }
    delay(60000UL);
    nextServer();
  }
}

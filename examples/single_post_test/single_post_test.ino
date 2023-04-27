#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
#define LED_PIN 13
// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";

const char userPublicKey[] = "BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8";
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

      deso.updateSinglePost(postHash, true, 0, 0, userPublicKey, false, &post1);
      post1.LikedByReader == true ? digitalWrite(LED_PIN, HIGH) : digitalWrite(LED_PIN, LOW);
      Serial.printf("PostHashHex:%s\n", post1.PostHashHex);
      Serial.printf("Body:%s\n", post1.Body);
      Serial.printf("LikeCount:%d\n", post1.LikeCount);
      Serial.printf("DiamondCount:%d\n", post1.DiamondCount);
      Serial.printf("CommentCount:%d\n", post1.CommentCount);
      Serial.printf("QuoteRepostCount:%d\n", post1.QuoteRepostCount);
      Serial.printf("LikedByReader:%d\n", post1.LikedByReader);
      Serial.println("======================");
    }
    delay(10000UL);
    nextServer();
  }
}

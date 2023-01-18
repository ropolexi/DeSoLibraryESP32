#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
#define RELAY1_PIN 13

// Fill in the username
const char username[] = "ropolexi";
const char postHash[] = "a6039c9a223d5fad11b05d54a152d0f77048a1552e06e7430697848db60abf71";
// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";

DeSoLib deso;
DeSoLib::Profile profile1;
DeSoLib::Post post1;

int server_index = 0;

void nextServer()
{
  server_index++; // try different nodes
  if (server_index >= deso.getMaxNodes())
    server_index = 0;
}

void setup()
{
  pinMode(RELAY1_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, HIGH);

  Serial.begin(9600);
  WiFi.enableSTA(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, wifi_pass);

  while (!WiFi.isConnected())
  {
    delay(1000);
    Serial.print(".");
  }
  deso.addNodePath("https://diamondapp.com", ISRG_Root_caRootCert);
  deso.addNodePath("https://node.deso.org", GTS_Root_caRootCert);

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
}

void loop()
{
  int top_holder_index = 0;
  if (WiFi.isConnected())
  {
    deso.updateTopHolders("", profile1.PublicKeyBase58Check, 2, &profile1);
    //if account owner has coins it is in index 0
    //descending order
    
    if (profile1.TopHodlersCoinsPerc[0] > profile1.TopHodlersCoinsPerc[1])
    {
      top_holder_index = 0;
      
    }
    else
    {
      for (int i = 0; i < 2; i++)
      {
        if (strlen(profile1.TopHodlersUserNames[i]) > 0)
        {
          if (strcmp(profile1.TopHodlersPublicKeyBase58Check[i], profile1.PublicKeyBase58Check) != 0)//other than owner
          {
            top_holder_index = i;
            break;
          }
        }
      }
    }
    Serial.printf("Top Holder:%s\n", profile1.TopHodlersUserNames[top_holder_index]);
    
    deso.updateSinglePost(postHash, true, 0, 0, profile1.TopHodlersPublicKeyBase58Check[top_holder_index], false, &post1);
    if (post1.LikedByReader == true)
    {
      digitalWrite(RELAY1_PIN, LOW);
      Serial.println("ON");
    }
    else
    {
      digitalWrite(RELAY1_PIN, HIGH);
      Serial.println("OFF");
    }
  }
  delay(30000UL);
}
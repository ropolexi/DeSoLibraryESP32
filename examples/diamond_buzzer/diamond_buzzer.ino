#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#define LIKE_PIN 18
#define BALANCE_PIN 15
#define BUZZER_PIN 19

// Fill in the username
const char username[] = "ropolexi";
// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";

DeSoLib deso;
DeSoLib::Profile profile1;
// DeSoLib::Post post1;
int diamond_count = 0;
int like_count = 0;
int server_index = 0;
double balance = 0;

void nextServer()
{
  server_index++; // try different nodes
  if (server_index >= deso.getMaxNodes())
    server_index = 0;
}

void setup()
{
  pinMode(LIKE_PIN, OUTPUT);
  pinMode(BALANCE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LIKE_PIN, LOW);
  digitalWrite(BALANCE_PIN, LOW);
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);

  digitalWrite(BUZZER_PIN, LOW);
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
  deso.addNodePath("https://desocialworld.com", GTS_Root_R1_caRootCert);
  deso.addNodePath("https://diamondapp.com", Baltimore_Root_caRootCert);
  deso.addNodePath("https://node.deso.org", GTS_Root_R1_caRootCert);

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
  if (WiFi.isConnected())
  {
    Serial.println("Updating");
    digitalWrite(LED_BUILTIN, HIGH);

    deso.updateLastNumPostsForPublicKey(profile1.PublicKeyBase58Check, 5, &profile1);

    deso.updateUsersBalance(profile1.PublicKeyBase58Check, &profile1);
    digitalWrite(LED_BUILTIN, LOW);

    if (profile1.lastNPostDiamonds > 0)
    {
      Serial.printf("DIAMOND:%d\n", profile1.lastNPostDiamonds);
      if (diamond_count != profile1.lastNPostDiamonds)
      {
        int change = profile1.lastNPostDiamonds - diamond_count;
        diamond_count = profile1.lastNPostDiamonds;
        if (change > 0)
        {
          for (int i = 0; i < change; i++)
          {
            digitalWrite(BUZZER_PIN, HIGH);
            delay(100);
            digitalWrite(BUZZER_PIN, LOW);
            delay(50);
          }
        }
      }
    }
    Serial.printf("LIKE:%d\n", profile1.lastNPostLikes);
    if (like_count != profile1.lastNPostLikes)
    {
      like_count = profile1.lastNPostLikes;
      digitalWrite(LIKE_PIN, HIGH);
    }
    double total = profile1.BalanceNanos + profile1.UnconfirmedBalanceNanos;
    Serial.printf("Balance:%f\n", total);
    if (total > 0)
    {
      if (total != balance)
      {
        balance = total;
        digitalWrite(BALANCE_PIN, HIGH);
      }
    }
    delay(2000);
    digitalWrite(BALANCE_PIN, LOW);
    digitalWrite(LIKE_PIN, LOW);
  }
  Serial.println("Done");
  delay(10000UL);
}

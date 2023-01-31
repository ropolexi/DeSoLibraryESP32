#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>

#define LIKE_1_PIN 4
#define LIKE_2_PIN 16
#define LIKE_3_PIN 17
#define LIKE_4_PIN 5
#define LIKE_5_PIN 18

#define DIAMOND_1_PIN 19
#define DIAMOND_2_PIN 21
#define DIAMOND_3_PIN 22
#define DIAMOND_4_PIN 23
#define DIAMOND_5_PIN 32

#define BALANCE_PIN 15
#define BUZZER_PIN 13

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

void like_led_test()
{
  digitalWrite(LIKE_1_PIN, HIGH);
  delay(100);
  digitalWrite(LIKE_2_PIN, HIGH);
  delay(100);
  digitalWrite(LIKE_3_PIN, HIGH);
  delay(100);
  digitalWrite(LIKE_4_PIN, HIGH);
  delay(100);
  digitalWrite(LIKE_5_PIN, HIGH);
  delay(100);
}

void diamond_led_clear()
{
  digitalWrite(DIAMOND_1_PIN, LOW);
  digitalWrite(DIAMOND_2_PIN, LOW);
  digitalWrite(DIAMOND_3_PIN, LOW);
  digitalWrite(DIAMOND_4_PIN, LOW);
  digitalWrite(DIAMOND_5_PIN, LOW);
}

void like_led_clear()
{
  digitalWrite(LIKE_1_PIN, LOW);
  digitalWrite(LIKE_2_PIN, LOW);
  digitalWrite(LIKE_3_PIN, LOW);
  digitalWrite(LIKE_4_PIN, LOW);
  digitalWrite(LIKE_5_PIN, LOW);
}
void diamond_led_test()
{
  digitalWrite(DIAMOND_1_PIN, HIGH);
  delay(100);
  digitalWrite(DIAMOND_2_PIN, HIGH);
  delay(100);
  digitalWrite(DIAMOND_3_PIN, HIGH);
  delay(100);
  digitalWrite(DIAMOND_4_PIN, HIGH);
  delay(100);
  digitalWrite(DIAMOND_5_PIN, HIGH);
  delay(100);
}

void diamond_led_indicate(int num)
{
  diamond_led_clear();
  switch (num)
  {
  case 0:
    break;
  case 1:
    digitalWrite(DIAMOND_1_PIN, HIGH);
    break;
  case 2:
    digitalWrite(DIAMOND_1_PIN, HIGH);
    digitalWrite(DIAMOND_2_PIN, HIGH);
    break;
  case 3:
    digitalWrite(DIAMOND_1_PIN, HIGH);
    digitalWrite(DIAMOND_2_PIN, HIGH);
    digitalWrite(DIAMOND_3_PIN, HIGH);
    break;
  case 4:
    digitalWrite(DIAMOND_1_PIN, HIGH);
    digitalWrite(DIAMOND_2_PIN, HIGH);
    digitalWrite(DIAMOND_3_PIN, HIGH);
    digitalWrite(DIAMOND_4_PIN, HIGH);
    break;
  case 5:
    digitalWrite(DIAMOND_1_PIN, HIGH);
    digitalWrite(DIAMOND_2_PIN, HIGH);
    digitalWrite(DIAMOND_3_PIN, HIGH);
    digitalWrite(DIAMOND_4_PIN, HIGH);
    digitalWrite(DIAMOND_5_PIN, HIGH);
    break;
  default:
    digitalWrite(DIAMOND_1_PIN, HIGH);
    digitalWrite(DIAMOND_2_PIN, HIGH);
    digitalWrite(DIAMOND_3_PIN, HIGH);
    digitalWrite(DIAMOND_4_PIN, HIGH);
    digitalWrite(DIAMOND_5_PIN, HIGH);
    break;
  }
}

void like_led_indicate(int num)
{
  like_led_clear();
  switch (num)
  {
  case 0:
    break;
  case 1:
    digitalWrite(LIKE_1_PIN, HIGH);
    break;
  case 2:
    digitalWrite(LIKE_1_PIN, HIGH);
    digitalWrite(LIKE_2_PIN, HIGH);
    break;
  case 3:
    digitalWrite(LIKE_1_PIN, HIGH);
    digitalWrite(LIKE_2_PIN, HIGH);
    digitalWrite(LIKE_3_PIN, HIGH);
    break;
  case 4:
    digitalWrite(LIKE_1_PIN, HIGH);
    digitalWrite(LIKE_2_PIN, HIGH);
    digitalWrite(LIKE_3_PIN, HIGH);
    digitalWrite(LIKE_4_PIN, HIGH);
    break;
  case 5:
    digitalWrite(LIKE_1_PIN, HIGH);
    digitalWrite(LIKE_2_PIN, HIGH);
    digitalWrite(LIKE_3_PIN, HIGH);
    digitalWrite(LIKE_4_PIN, HIGH);
    digitalWrite(LIKE_5_PIN, HIGH);
    break;
  default:
    digitalWrite(LIKE_1_PIN, HIGH);
    digitalWrite(LIKE_2_PIN, HIGH);
    digitalWrite(LIKE_3_PIN, HIGH);
    digitalWrite(LIKE_4_PIN, HIGH);
    digitalWrite(LIKE_5_PIN, HIGH);
    break;
  }
}

void setup()
{
  pinMode(LIKE_1_PIN, OUTPUT);
  pinMode(LIKE_2_PIN, OUTPUT);
  pinMode(LIKE_3_PIN, OUTPUT);
  pinMode(LIKE_4_PIN, OUTPUT);
  pinMode(LIKE_5_PIN, OUTPUT);

  pinMode(DIAMOND_1_PIN, OUTPUT);
  pinMode(DIAMOND_2_PIN, OUTPUT);
  pinMode(DIAMOND_3_PIN, OUTPUT);
  pinMode(DIAMOND_4_PIN, OUTPUT);
  pinMode(DIAMOND_5_PIN, OUTPUT);

  pinMode(BALANCE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  like_led_clear();
  diamond_led_clear();
  like_led_test();
  diamond_led_test();
  delay(500);
  like_led_clear();
  diamond_led_clear();

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
  deso.addNodePath("https://desocialworld.com", ISRG_Root_caRootCert);
  deso.addNodePath("https://diamondapp.com",Baltimore_Root_caRootCert );
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
  if (WiFi.isConnected())
  {
    Serial.println("Updating");
    digitalWrite(LED_BUILTIN, HIGH);
    profile1.lastNPostDiamonds = -1;
    profile1.lastNPostLikes = -1;
    deso.updateLastNumPostsForPublicKey(profile1.PublicKeyBase58Check, 1, &profile1);

    deso.updateUsersBalance(profile1.PublicKeyBase58Check, &profile1);
    digitalWrite(LED_BUILTIN, LOW);

    if (profile1.lastNPostDiamonds >= 0)
    {
      Serial.printf("DIAMOND:%d\n", profile1.lastNPostDiamonds);
      if (diamond_count != profile1.lastNPostDiamonds)
      {
        int change = profile1.lastNPostDiamonds - diamond_count;
        diamond_count = profile1.lastNPostDiamonds;
        diamond_led_indicate(diamond_count);

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
    if (profile1.lastNPostLikes >= 0)
    {
      Serial.printf("LIKE:%d\n", profile1.lastNPostLikes);
      if (like_count != profile1.lastNPostLikes)
      {
        like_count = profile1.lastNPostLikes;
        like_led_indicate(like_count);
      }
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
    // digitalWrite(LIKE_PIN, LOW);
  }
  Serial.println("Done");
  delay(15000UL);
}
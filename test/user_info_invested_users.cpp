
#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "time.h"
#include "sntp.h"

#define UPDATE_INTERVAL 60
// Fill in timezone in seconds
const long gmtOffset_sec = 19800; //+5:30
const int daylightOffset_sec = 0;

// Fill in the username
const char username[] = "PurpleVan_Arduino";
// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";

DeSoLib deso;
int server_index = 0;
unsigned long timer_refresh = -60000UL;
void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

void nextServer()
{
  server_index++; // try different node
  if (server_index >= deso.getMaxNodes())
    server_index = 0;
}

void skipNotWorkingNode()
{
  do
  {
    deso.selectDefaultNode(server_index);
    //Serial.printf("Selecting node %s\n", deso.getSelectedNodeUrl());
    deso.updateNodeHealthCheck();
    if (!deso.getSelectedNodeStatus())
    {
      Serial.printf("%s Node error!\n", deso.getSelectedNodeUrl());
      nextServer();
    }
  } while (!deso.getSelectedNodeStatus());
}



void getExchangeRates()
{
  deso.updateExchangeRates();
  Serial.print("DeSo Coin Value:\t\t$");
  Serial.println(deso.USDCentsPerBitCloutExchangeRate / 100.0);
  Serial.print("BTC:\t\t\t\t$");
  Serial.println(deso.USDCentsPerBitcoinExchangeRate / 100.0);
}

void getProfile(const char *username,bool save=false)
{
  DeSoLib::Profile profile1;
  deso.updateSingleProfile(username, "", &profile1); // search by username or public key
  Serial.println();
  Serial.print("Username:\t\t\t");
  Serial.println(profile1.Username);
  Serial.print("PublicKey:\t\t\t");
  Serial.println(profile1.PublicKeyBase58Check);
  Serial.print("Creator Coin Price:\t\t$");
  double coinPriceUSDCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.CoinPriceBitCloutNanos / 1000000000.0);
  Serial.println(coinPriceUSDCents / 100.0);
  Serial.print("Wallet Balance:\t\t\t$");
  double balanceCents = deso.USDCentsPerBitCloutExchangeRate * ((profile1.BalanceNanos+profile1.UnconfirmedBalanceNanos) / 1000000000.0);
  Serial.println(balanceCents / 100.0);
  Serial.println("====Creator Coins holding====");
  
  deso.updateHodleAssetBalance("", profile1.PublicKeyBase58Check, &profile1,save);
  

  Serial.println("========");
  Serial.print("Total HODL assets :\t\t");
  Serial.println(profile1.TotalHodleNum);
  Serial.print("Total HODL Asset Balance:\t$");
  double assetsValue = (profile1.TotalHODLBalanceClout * deso.USDCentsPerBitCloutExchangeRate) / 100.0;
  Serial.println(assetsValue);

  deso.updateLastNumPostsForPublicKey(profile1.PublicKeyBase58Check, 5, &profile1);
  Serial.print("Last 5 Post Likes:\t\t");
  Serial.println(profile1.lastNPostLikes);
  Serial.print("Last 5 Post Diamonds:\t\t");
  Serial.println(profile1.lastNPostDiamonds);

  deso.updateLastNumPostsForPublicKey(profile1.PublicKeyBase58Check,1, &profile1);
  Serial.print("Last Post Likes:\t\t");
  Serial.println(profile1.lastNPostLikes);
  Serial.print("Last Post Diamonds:\t\t");
  Serial.println(profile1.lastNPostDiamonds);

  deso.clearTopHodlersUserNames(&profile1);

  deso.updateTopHolders("", profile1.PublicKeyBase58Check, 10, &profile1);
  Serial.println("\nTop 10 Creator Coins Hodlers:");
  for (int i = 0; i < 10; i++)
  {
    if (strlen(profile1.TopHodlersUserNames[i]) > 0)
    {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(profile1.TopHodlersUserNames[i]);
      // Serial.print(" - ");
      // Serial.print(profile1.TopHodlersCoins[i]);
      Serial.print(" - ");
      Serial.print(profile1.TopHodlersCoinsPerc[i]);
      Serial.println("%");
    }
  }
  Serial.println("\n");
  
}
void setup()
{

  Serial.begin(9600);
  Serial.println();
  delay(1000);
  Serial.println("*** DeSo Dashboard on ESP32 ***");
  sntp_set_time_sync_notification_cb(timeavailable);
  // sntp_servermode_dhcp(1);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  Serial.setDebugOutput(false);
  WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, wifi_pass);
  unsigned long timerout_wifi = millis();
  Serial.print("Wifi connecting.");
  while (!WiFi.isConnected() && millis() - timerout_wifi < 10000)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWifi connected.");
  Serial.print("Time syncing...");
  timerout_wifi = millis();
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo) && millis() - timerout_wifi < 10000)
  {
    delay(1000);
  }

  deso.addNodePath("https://desocialworld.com", GTS_Root_R1_caRootCert);
  deso.addNodePath("https://diamondapp.com", Baltimore_Root_caRootCert);
  deso.addNodePath("https://node.deso.org", GTS_Root_R1_caRootCert);
  deso.selectDefaultNode(0);

  getExchangeRates();
  getProfile(username,true);
  Serial.println("*******");
  for(int i=0;i<deso.users.size();i++){
    if(strcmp(deso.users[i].username,username)==0){
      continue;
    }
    Serial.print("Getting User data ");+-
    Serial.println(deso.users[i].username);
    getProfile(deso.users[i].username,false);
  }
  Serial.println("*******");
}
void loop()
{
  while (true)
  {
  
  }
}

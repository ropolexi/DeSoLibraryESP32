#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
DeSoLib deso;
const char ssid[] = "";
const char wifi_pass[] = "";
int server_index = 0;
void setup()
{
  esp_init();
  //You may need to add root certificate if you want to make sure the site is correct
  //Since this example is just for display purposes it is ommited here
  deso.addNodePath("https://bitclout.com", "");
  deso.addNodePath("https://nachoaverage.com", "");
  deso.addNodePath("https://members.giftclout.com", "");
  deso.addNodePath("https://tijn.club", "");
  deso.selectDefaultNode(0);
}
void loop()
{
  while (true)
  {
    if (WiFi.isConnected())
    {
      skipNotWorkingNode();
      getExchangeRates();
      getProfile("ropolexi");
    }
    delay(10000);   //delay 10 seconds
    server_index++; //select next node
    if (server_index >= deso.getMaxNodes())
      server_index = 0;
  }
}
void getExchangeRates()
{
  deso.updateExchangeRates();
  Serial.print("DeSo Coin Value: $");
  Serial.println(deso.USDCentsPerBitCloutExchangeRate / 100.0);
  Serial.print("BTC: $");
  Serial.println(deso.USDCentsPerBitcoinExchangeRate / 100.0);
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

void getProfile(const char *username)
{
  Serial.println("=======Profile========");
  DeSoLib::Profile profile1;
  delay(1000);
  deso.updateSingleProfile(username, "", &profile1); //search by username or public key
  //deso.updateSingleProfile("", "BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8", &profile1);

  Serial.print("Username: ");
  Serial.println(profile1.Username);
  Serial.print("PublicKey: ");
  Serial.println(profile1.PublicKeyBase58Check);
  Serial.print("Creator Coin Price: $");
  double coinPriceUSDCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.CoinPriceBitCloutNanos / 1000000000.0);
  Serial.println(coinPriceUSDCents / 100.0);
  deso.updateUsersStateless(profile1.PublicKeyBase58Check, false, &profile1);
  Serial.print("Wallet Balance:");
  double balanceCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.BalanceNanos / 1000000000.0);
  Serial.println(balanceCents / 100.0);
  Serial.print("Total HODL assets : ");
  Serial.println(profile1.TotalHodleNum);
  Serial.print("Total HODL Asset Balance: $");
  double assetsValue = (profile1.TotalHODLBalanceClout * deso.USDCentsPerBitCloutExchangeRate) / 100.0;
  Serial.println(assetsValue);
  deso.updateLastPostForPublicKey(profile1.PublicKeyBase58Check, &profile1);
  Serial.print("Last Post Likes: ");
  Serial.println(profile1.lastPostLikes);
  Serial.print("Last Post Diamonds: ");
  Serial.println(profile1.lastPostDiamonds);
  Serial.println("======================");
}
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
}
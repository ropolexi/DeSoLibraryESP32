#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"

DeSoLib deso;

const char ssid[] = "";
const char wifi_pass[] = "";
bool skipHodlings = false;

void getProfile(const char *Username)
{
  DeSoLib::Profile profile1;
  Serial.println("\n=======Profile========");
  deso.updateSingleProfile(Username, "", &profile1); //search by username or public key
  //deso.updateSingleProfile("", "BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8", &profile1);
  if (strcmp(profile1.PublicKeyBase58Check, "NULL") != 0)
  {
    Serial.print("Username: ");
    Serial.println(profile1.Username);
    Serial.print("PublicKey: ");
    Serial.println(profile1.PublicKeyBase58Check);
    Serial.print("Creator Coin Price: $");
    double coinPriceUSDCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.CoinPriceBitCloutNanos / 1000000000.0);
    Serial.println(coinPriceUSDCents / 100.0);
    deso.updateUsersStateless(profile1.PublicKeyBase58Check, skipHodlings, &profile1);

    double balanceCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.BalanceNanos / 1000000000.0);
    if (balanceCents != 0)//if UsersStateless is huge balance may not update
    {
      Serial.print("Wallet Balance: $");
      Serial.println(balanceCents / 100.0);
    }
    if (skipHodlings == false)
    {
      Serial.print("Total assets : ");
      Serial.println(profile1.TotalHodleNum);
      Serial.print("Total Asset Balance: $");
      double assetsValue = (profile1.TotalHODLBalanceClout * deso.USDCentsPerBitCloutExchangeRate) / 100.0;
      Serial.println(assetsValue);
    }
    deso.clearTopHodlersUserNames(&profile1);
    deso.updateHodlersForPublicKey("", profile1.PublicKeyBase58Check, 10, &profile1);
    Serial.println("\nTop 10 Hodlers:");
    Serial.println("**********************");
    for (int i = 0; i < 10; i++)
    {
      if (strlen(profile1.TopHodlersUserNames[i]) > 0)
      {
        Serial.print(i + 1);
        Serial.print(":");
        Serial.println(profile1.TopHodlersUserNames[i]);
      }
    }
    Serial.println("**********************");
  }
  else
  {
    Serial.println("");
    Serial.print(profile1.Username);
    Serial.print(" not found");
  }
  Serial.println("\n======================");
}
void setup()
{
  // put your setup code here, to run once:
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
  deso.addNodePath("https://bitclout.com", bitclout_caRootCert);
  deso.addNodePath("https://nachoaverage.com", nachoaverage_caRootCert);
  deso.addNodePath("https://members.giftclout.com", giftclout_caRootCert);
  deso.selectDefaultNode(0);
  Serial.println("\n\nDevice: ESP32-WROOM-32");
  Serial.println("**** Welcome to DeSo ******");
}

void loop()
{
  static int server_index = 0;

  if (WiFi.isConnected())
  {
    do
    {
      deso.selectDefaultNode(server_index);
      deso.updateNodeHealthCheck();
      Serial.println();
      Serial.print("DeSo Node: ");
      Serial.println(deso.getSelectedNodeUrl());
      Serial.print("Node Status: ");
      if (deso.getSelectedNodeStatus())
      {
        Serial.println("Synced OK");
      }
      else
      {
        Serial.println("Not Synced");
      }

      if (!deso.getSelectedNodeStatus())
      {
        server_index++;
        if (server_index >= deso.getMaxNodes())
          server_index = 0;
      }

    } while (!deso.getSelectedNodeStatus());

    deso.updateExchangeRates();
    Serial.print("DeSo Coin Value: $");
    Serial.println(deso.USDCentsPerBitCloutExchangeRate / 100.0);
    Serial.print("BTC :$");
    Serial.println(deso.USDCentsPerBitcoinExchangeRate / 100.0);

    getProfile("ropolexi");
  
  }
  delay(20000UL);
  server_index++; //try different nodes
  if (server_index >= deso.getMaxNodes())
  {
    server_index = 0;
  }
}
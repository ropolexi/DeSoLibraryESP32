#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"

//Fill in the ssid and password
const char ssid[]="";
const char wifi_pass[]="";

DeSoLib deso;

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
  deso.addNodePath("https://members.giftclout.com",giftclout_caRootCert);
  deso.selectDefaultNode(0);
}

void loop()
{
  static int server_index = 0;

  if (WiFi.isConnected())
  {
    do{
      deso.selectDefaultNode(server_index);
      deso.updateNodeHealthCheck();
      if(!deso.getSelectedNodeStatus()){
        server_index++;
        if(server_index >= deso.getMaxNodes()) server_index=0; 
      }
       
    }while (!deso.getSelectedNodeStatus());

    Serial.println();
    Serial.print("DeSo Node: ");
    Serial.println(deso.getSelectedNodeUrl());

    deso.updateExchangeRates();
    Serial.print("DeSo Coin Value: $");
    Serial.println(deso.USDCentsPerBitCloutExchangeRate / 100.0);
    //Serial.println("BTC (USD):");
    //Serial.println(deso.USDCentsPerBitcoinExchangeRate/100.0);
    Serial.println("=======Profile========");
    DeSoLib::Profile profile1;
    delay(1000);
    deso.updateSingleProfile("ropolexi", "" ,&profile1);//search by username or public key
    //deso.updateSingleProfile("", "BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8", &profile1);

    Serial.print("Username: ");
    Serial.println(profile1.Username);
    Serial.print("PublicKey: ");
    Serial.println(profile1.PublicKeyBase58Check);
    Serial.print("Creator Coin Price: $");
    double coinPriceUSDCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.CoinPriceBitCloutNanos / 1000000000.0);
    Serial.println(coinPriceUSDCents / 100.0);
    deso.updateUsersStateless("BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8", true, &profile1);
    Serial.print("Wallet Balance:");
    double balanceCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.BalanceNanos / 1000000000.0);
    Serial.println(balanceCents / 100.0);
    Serial.print("Total HODLE assets : ");
    Serial.println(profile1.TotalHodleNum);
    Serial.print("Total HODLE Asset Balance: $");
    double assetsValue=(profile1.TotalHODLBalanceClout*deso.USDCentsPerBitCloutExchangeRate)/100.0;
    Serial.println(assetsValue);
    Serial.println("======================");
  }
  delay(10000UL);
  server_index++;//try different nodes
  if(server_index >= deso.getMaxNodes()) server_index=0; 
}
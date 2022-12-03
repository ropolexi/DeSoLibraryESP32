//Using v0 API to get balance (not the best solution)
#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
DeSoLib deso;

//fill in wifi details
const char ssid[] = "";
const char wifi_pass[] = "";

DeSoLib::Profile profile1; //variable to store user profile details

//initialize wifi and serial port
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
  Serial.println("Connected to Wifi.");
}

void setup()
{
  esp_init();
  deso.addNodePath("https://bitclout.com", bitclout_caRootCert);
  deso.selectDefaultNode(0);
  deso.updateSingleProfile("ropolexi", "", &profile1); //get the public key from username

  //Or you could use public key to get data
  //deso.updateSingleProfile("","BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8", &profile1);
}

void loop()
{
  if (WiFi.isConnected())
  {
    Serial.println("Refreshing balance..");
    deso.updateExchangeRates();                                                //update exchange rates to get balance in $ unit
    deso.updateUsersStateless(profile1.PublicKeyBase58Check, true, &profile1); //update user state
    Serial.print("Wallet Balance: $");
    double balanceCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.BalanceNanos / 1000000000.0);
    Serial.println(balanceCents / 100.0); //balance in $
  }
  delay(5000); //delay 5 seconds
}
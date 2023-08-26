// check all api end points
#include "Arduino.h"
#include "DeSoLib.h"
DeSoLib deso;
DeSoLib::Profile profile1;
DeSoLib::Post post1;
const char wifi_ssid[] = "";
const char wifi_pass[] = "";
RTC_DATA_ATTR int bootCount = 0;

void checkSuccess(int status)
{
  if (status)
  {
    Serial.println("\n=>[OK]");
  }
  else
  {
    Serial.println("\n=>[ERROR]");
  }
}
void setup()
{
  Serial.begin(9600);
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  WiFi.begin(wifi_ssid, wifi_pass);
  deso.addNodePath("https://node.deso.org", GTS_Root_R1_caRootCert);
  while (!WiFi.isConnected()){
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  deso.selectDefaultNode(0);

  Serial.printf("Node: %s\n", deso.getSelectedNodeUrl());

  Serial.printf("API :%s ", deso.RoutePathHealthCheck);
  checkSuccess(deso.updateNodeHealthCheck());

  Serial.printf("API :%s ", deso.ExchangeRateRoute);
  checkSuccess(deso.updateExchangeRates());

  Serial.printf("API :%s ", deso.RoutePathGetAppState);
  checkSuccess(deso.getAppState());

  Serial.printf("API :%s ", deso.RoutePathGetSingleProfile);
  checkSuccess(deso.updateSingleProfile("deso","",&profile1));

  Serial.printf("API :%s ", deso.RoutePathGetPostsStateless);
  checkSuccess(deso.updatePostsStateless("",profile1.PublicKeyBase58Check,10,false,60));

  Serial.printf("API :%s ", deso.RoutePathGetSinglePost);
  checkSuccess(deso.updateSinglePost("dda17ba3ad2930dfe20a4f50329ef8158efb00e350d00b1b6e897da7897b7bba",false,0,0,profile1.PublicKeyBase58Check,false,&post1));
  
  Serial.printf("API :%s ", deso.RoutePathGetPostsForPublicKey);
  checkSuccess(deso.updateLastNumPostsForPublicKey(profile1.PublicKeyBase58Check,5,&profile1));

  Serial.printf("API :%s ", deso.RoutePathGetHodlersForPublicKey);
  checkSuccess(deso.updateHodleAssetBalance("deso","",&profile1,false));

  int count=0;
  Serial.printf("API :%s ", deso.RoutePathCountPostAssociationsSingle);
  checkSuccess(deso.countPostAssociationSingle(profile1.PublicKeyBase58Check,"dda17ba3ad2930dfe20a4f50329ef8158efb00e350d00b1b6e897da7897b7bba","LIKE",&count));
  
  DeSoLib::ReactionCount reactions;
  Serial.printf("API :%s ", deso.RoutePathCountPostAssociations);
  checkSuccess(deso.countPostAssociation(profile1.PublicKeyBase58Check,"dda17ba3ad2930dfe20a4f50329ef8158efb00e350d00b1b6e897da7897b7bba",&reactions));

  char owner[56];
  Serial.printf("API :%s ", deso.RoutePathNFTEntriesForNFTPost);
  checkSuccess(deso.getNFTEntriesForNFTPost("1de6f37e03b7da18a7a79b927a5716fe6c1913b1fd7fe51d21d3b2f963aa7a47",1,owner));
 

  esp_sleep_enable_timer_wakeup(20000000ULL);
  WiFi.disconnect(true);
  delay(100);
  Serial.flush();

  // esp_deep_sleep_start();
}

void loop()
{
  
  
  esp_light_sleep_start();
}
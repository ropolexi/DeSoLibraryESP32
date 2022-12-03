
#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "time.h"
#include "sntp.h"

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";

//Fill in timezone in seconds
const long  gmtOffset_sec = 19800;//+5:30 
const int   daylightOffset_sec = 0;

//Fill in the ssid and password
const char ssid[]="";
const char wifi_pass[]="";

DeSoLib deso;
int server_index = 0;
unsigned long timer_refresh = -30000;
void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
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
    server_index++; //try different nodes
    if (server_index >= deso.getMaxNodes())
        server_index = 0;
}

void skipNotWorkingNode()
{
    do
    {       
        deso.selectDefaultNode(server_index);
        Serial.printf("selecting node %s\n",deso.getSelectedNodeUrl());
        Serial.println("update node health check");
        deso.updateNodeHealthCheck();
        if (!deso.getSelectedNodeStatus())
        {
            Serial.printf("%s Node error!\n",deso.getSelectedNodeUrl());
            nextServer();
        }
    } while (!deso.getSelectedNodeStatus());
}


void setup()
{

    Serial.begin(9600);
    sntp_set_time_sync_notification_cb( timeavailable );
    sntp_servermode_dhcp(1);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
    Serial.setDebugOutput(false);
    WiFi.mode(WIFI_AP_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid, wifi_pass);
    unsigned long timerout_wifi = millis();
    while (!WiFi.isConnected() && millis() - timerout_wifi < 10000)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("Wifi connected.");

    deso.addNodePath("https://bitclout.com", "");
    deso.addNodePath("https://node.deso.org", "");
    deso.addNodePath("https://love4src.com", "");
    deso.selectDefaultNode(0);
    
}

void loop()
{
    while (true)
    {
        if (millis() - timer_refresh > 30000)//update every 30 seconds
        {
             timer_refresh = millis();

            if (WiFi.isConnected())
            {
                    skipNotWorkingNode();
                    deso.updatePostsStateless("","BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8",5,false,30);
                    nextServer();
            }
            else
            {
                debug_print("WIFI reconnecting...");
            }
        }
    }
}
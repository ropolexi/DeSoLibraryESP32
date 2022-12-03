/*
Initital Setup - Connect pc or phone to the esp32 device via wifi access point,
default parameters are
char ssid_custom_ap[]="TestAP";
char pass_custom_ap[]="desorocks"

In the web browser goto ip address 192.168.4.1
change the wifi settings or profile settings and submit

if username has large data parameters such as following and holding assets
the device may restart due to low memory , and to prevent endless loop of restarts
the holdings output will be disabled. 

Update Users Holdings can be disabled for those user profiles that are too large to handle
by ESP32, if the device restarts for a given profile , update the username and disable the
Update Users holdings from updating.
*/
#include <Arduino.h>
#include "DeSoLib.h"
#include "EEPROM.h"
#include <WiFi.h>
//#include "cert.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "customchars.h"
#include <WebServer.h>
#include "ap.h"
WebServer server(80);
#define I2C_SDA 21
#define I2C_SCL 22

// Set the LCD address to 0x27 for a 16 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 16, 4);

//Fill in the custom ESP AP ssid and password

char ssid_custom_ap[] = "TestAP";
char pass_custom_ap[] = "desorocks";

char ssid_ap[20];
char pass_ap[20];
char username[20];
DeSoLib deso;
DeSoLib::Profile profile1;
int server_index = 0;
#define CRASH_STATUS_ADD 500
#define UPDATE_USERSTATELESS_ADD 499
bool updateUsersStateless = false;
unsigned long timer_refresh = 30000;

void handleRoot()
{
    server.send(200, "text/html", postForms);
}
void wifihandleForm()
{
    String msg;
    if (server.method() != HTTP_POST)
    {
        server.send(405, "text/plain", "Method Not Allowed");
    }
    else
    {
        for (uint8_t i = 0; i < server.args(); i++)
        {
            if (server.argName(i) == "SSID")
            {
                String ssid_str = server.arg(i);
                if (ssid_str.length() != 0)
                {
                    if (!ssid_str.equals(String(ssid_ap)))
                    {
                        sprintf(ssid_ap, "%s", ssid_str.c_str());
                        msg += "ssid changed,";

                        EEPROM.put(0, ssid_ap);
                    }
                }
            }
            else if (server.argName(i) == "PASS")
            {
                String pass_str = server.arg(i);

                if (pass_str.length() != 0)
                {
                    if (!pass_str.equals(String(pass_ap)))
                    {
                        sprintf(pass_ap, "%s", pass_str.c_str());
                        msg += "wifi password changed,";

                        EEPROM.put(0 + sizeof(ssid_ap), pass_ap);
                    }
                }
            }
        }
        msg += "restarting....";
        EEPROM.commit();
        server.send(200, "text/html", msg);
        delay(100);
        ESP.restart();
    }
}

void profilehandleForm()
{
    String msg;

    String header = "<head><meta name='viewport' content='width=device-width, initial-scale=1.0'></head>[<a href='/'>Home</a>]<br>";
    if (server.method() != HTTP_POST)
    {
        server.send(405, "text/plain", "Method Not Allowed");
    }
    else
    {
        for (uint8_t i = 0; i < server.args(); i++)
        {
            if (server.argName(i) == "username")
            {
                String username_str = server.arg(i);
                if (username_str.length() != 0)
                {
                    if (!username_str.equals(String(username)))
                    {
                        int status = deso.updateSingleProfile(username_str.c_str(), "", &profile1);
                        if (status)
                        {
                            sprintf(username, "%s", username_str.c_str());
                            msg += "username changed to ";
                            msg += String(username);
                            msg += ",";
                            EEPROM.put(0 + sizeof(ssid_ap) + sizeof(pass_ap), username);
                            profile1.TotalHODLBalanceClout=0;
                            timer_refresh=millis()+30000UL;
                        }
                        else
                        {
                            if (WiFi.isConnected())
                            {
                                msg += "Error:can not use this ";
                                msg += String(username_str);
                                msg += " username,";
                            }
                            else
                            {
                                msg += "Error:no WIFI ";
                            }
                            //EEPROM.put(0 + sizeof(ssid_ap) + sizeof(pass_ap), "");
                        }
                    }
                }
            }
            if (server.argName(i) == "usersStateless")
            {
                String usersStateless_str = server.arg(i);
                if (usersStateless_str.length() != 0)
                {
                    if (usersStateless_str.equals("1"))
                    {
                        updateUsersStateless = true;
                        EEPROM.put(UPDATE_USERSTATELESS_ADD, (byte)1);
                    }
                    else
                    {
                        updateUsersStateless = false;
                        EEPROM.put(UPDATE_USERSTATELESS_ADD, (byte)0);
                    }
                }
            }
        }

        EEPROM.commit();
        msg += "updated.";

        server.send(200, "text/html", header + msg);
        delay(100);
    }
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
        deso.updateNodeHealthCheck();
        if (!deso.getSelectedNodeStatus())
        {
            nextServer();
        }
    } while (!deso.getSelectedNodeStatus());
    Serial.print("\nDeSo Node: ");
    Serial.println(deso.getSelectedNodeUrl());
}

void ap_server_init()
{
    IPAddress myIP = WiFi.softAPIP();
    WiFi.softAP(ssid_custom_ap, pass_custom_ap);
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", handleRoot);
    server.on("/wifi/", wifihandleForm);
    server.on("/profile/", profilehandleForm);
    server.begin();
}

void restore_data()
{
    EEPROM.begin(512);
    EEPROM.get(0, ssid_ap);
    EEPROM.get(0 + sizeof(ssid_ap), pass_ap);
    EEPROM.get(0 + sizeof(ssid_ap) + sizeof(pass_ap), username);
    Serial.print("\nSSID:");
    Serial.println(ssid_ap);

    Serial.print("\nUsername:");
    Serial.println(username);

    EEPROM.get(UPDATE_USERSTATELESS_ADD, updateUsersStateless);
    Serial.print("\nUpdate Users Stateless:");
    Serial.println(updateUsersStateless);
}

void lcd_init()
{
    Wire.begin(I2C_SDA, I2C_SCL);
    lcd.begin();
    lcd.backlight();
    lcd.createChar(0, deso_logo); //custom char for deso logo
    lcd.createChar(1, diamond);   //custom char for diomand icon
    lcd.createChar(2, heart);     //custom char for like icon
    lcd.clear();
    lcd.print("DeSo Dashbaord");
    lcd.setCursor(0, 1);
    lcd.print("(Bitclout)");
}
void setup()
{

    Serial.begin(9600);
    Serial.setDebugOutput(false);
    WiFi.mode(WIFI_AP_STA);
    WiFi.setAutoReconnect(true);
    lcd_init();
    restore_data();
    ap_server_init();
    WiFi.begin(ssid_ap, pass_ap);
    unsigned long timerout_wifi = millis();
    while (!WiFi.isConnected() && millis() - timerout_wifi < 10000)
    {
        delay(1000);
        Serial.print(".");
    }
    //without root cert ,do not use this method for critical application
    //not sending any private keys,for dashboards it is fine,
    deso.addNodePath("https://bitclout.com", "");
    deso.addNodePath("https://nachoaverage.com", "");
    deso.addNodePath("https://members.giftclout.com", "");
    deso.addNodePath("https://supernovas.app", "");
    deso.addNodePath("https://love4src.com", "");
    deso.addNodePath("https://stetnode.com", "");
    deso.addNodePath("https://bitcloutespañol.com", "");
    deso.addNodePath("https://desocial.nl", "");

    deso.selectDefaultNode(0);
    lcd.setCursor(-4, 3);
    lcd.print("Updating..");
    byte crash_status;
    EEPROM.get(CRASH_STATUS_ADD, crash_status);
    if (crash_status == 1)
    {
        //disable holdings if device crash due to not enough memory for the current username
        //EEPROM.put(0 + sizeof(ssid_ap) + sizeof(pass_ap), "");
        updateUsersStateless=false;
        EEPROM.put(UPDATE_USERSTATELESS_ADD, (byte)0);
        EEPROM.commit();
    }
    EEPROM.put(CRASH_STATUS_ADD, (byte)1);
    EEPROM.commit();
}
void updateDisplay()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    char str_url[17];
    strncpy(str_url, &deso.getSelectedNodeUrl()[0] + 8, 16); //limit to 16 charactors
    lcd.print(str_url);

    lcd.setCursor(0, 1);
    lcd.print("D:$");
    double temp = deso.USDCentsPerBitCloutExchangeRate / 100.0;
    lcd.print(temp, 0);

    double coinPriceUSDCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.CoinPriceBitCloutNanos / 1000000000.0);
    temp = coinPriceUSDCents / 100.0;
    lcd.setCursor(0 - 4, 2);
    lcd.print("        ");
    lcd.setCursor(0 - 4, 2);
    lcd.print("C:$");
    if(temp<1000){
    lcd.print(temp, 1);
    }else{
        lcd.print(temp/1000.0, 1);
        lcd.print("k");
    }

    double balanceCents = deso.USDCentsPerBitCloutExchangeRate * ((profile1.BalanceNanos+profile1.UnconfirmedBalanceNanos) / 1000000000.0);
    temp = balanceCents / 100.0;
    lcd.setCursor(8 - 4, 2);
    lcd.print("        ");
    lcd.setCursor(8 - 4, 2);
    lcd.print("B:$");
    if(temp<1000){
    lcd.print(temp, 1);
    }else{
        lcd.print(temp/1000.0, 1);
        lcd.print("k");
    }

    double assetsValue = (profile1.TotalHODLBalanceClout * deso.USDCentsPerBitCloutExchangeRate) / 100.0;
    lcd.setCursor(8, 1);
    temp = assetsValue;
    lcd.print("H:$");
    lcd.print(temp, 1);

    lcd.setCursor(-4, 3);
    lcd.write(2);
    lcd.print(" ");
    lcd.print(profile1.lastNPostLikes);

    lcd.setCursor(-4 + 8, 3);
    lcd.write(1);
    lcd.print(" ");
    lcd.print(profile1.lastNPostDiamonds);
}
void loop()
{
    static bool username_show = false;

    while (true)
    {
        server.handleClient();
        if (millis() - timer_refresh > 30000)//update every 30 seconds
        {

            if (WiFi.isConnected())
            {
                if (strlen(username) < 1)
                {
                    timer_refresh = millis();
                    lcd.clear();
                    lcd.setCursor(-4, 3);
                    lcd.print("No User");
                    timer_refresh = millis();
                }
                else
                {
                    Serial.println(username);
                    skipNotWorkingNode();
                    Serial.println("exchange...");
                    if (!deso.updateExchangeRates())
                    {
                        Serial.println("exchange error!");
                        nextServer();
                        continue;
                    }
                    Serial.println("updateSingleProfile...");

                    int status = deso.updateSingleProfile(username, "", &profile1);
                    if (!status)
                    {
                        Serial.println("single profile error!");
                        lcd.clear();
                        lcd.setCursor(-4, 3);
                        lcd.print("No User");
                        nextServer();
                        continue;
                    }
                    if (updateUsersStateless)
                    {
                        Serial.println("updateUsersStateless...");
                        status = deso.updateUsersStateless(profile1.PublicKeyBase58Check, false, &profile1);
                        if (!status)
                        {
                            Serial.println("user stateless error!");
                            nextServer();
                            continue;
                        }
                    }
                    else
                    {
                        Serial.println("updateUsersBalance...");
                        status = deso.updateUsersBalance(profile1.PublicKeyBase58Check, &profile1);
                        if (!status)
                        {
                            Serial.println("user balance error!");
                            nextServer();
                            continue;
                        }
                    }

                    Serial.println("updateLastNumPostsForPublicKey...");

                    status = deso.updateLastNumPostsForPublicKey(profile1.PublicKeyBase58Check, 5, &profile1);
                    if (!status)
                    {
                        Serial.println("update LastNum Posts For PublicKey error!");
                        nextServer();
                        continue;
                    }

                    updateDisplay();
                    nextServer();
                    username_show = true;
                    timer_refresh = millis();
                    EEPROM.put(CRASH_STATUS_ADD, (byte)0);
                    EEPROM.commit();
                }
            }
            else
            {
                lcd.clear();
                lcd.setCursor(-4, 3);
                lcd.print("No WiFi");

                debug_print("WIFI reconnecting...");

                WiFi.begin(ssid_ap, pass_ap);

                unsigned long timeout_wifi = millis();
                while (WiFi.isConnected() != true && millis() - timeout_wifi < 10000)
                {
                    server.handleClient();
                    delay(10);
                    //Serial.print(".");
                }
                if (WiFi.isConnected())
                {
                    timer_refresh = millis() + 10000 + 1;
                }
                else
                {
                    timer_refresh = millis();
                }
            }
        }

        if (millis() - timer_refresh > 2000 && username_show)
        { //show uusername after 2 seconds displaying other info
            username_show = false;
            lcd.setCursor(0, 0);
            lcd.print("                ");
            lcd.setCursor(0, 0);
            char str_url[17];
            strncpy(str_url, username, 16); //limit to 16 charactors
            lcd.print(str_url);
        }
    }
}
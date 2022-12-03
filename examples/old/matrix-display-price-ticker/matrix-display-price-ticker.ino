#include <Arduino.h>
#include <MD_MAX72xx.h>  //  https://github.com/MajicDesigns/MD_MAX72XX/
#include <SPI.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
DeSoLib deso;
const char ssid[] = "";
const char wifi_pass[] = "";

DeSoLib::Profile profile1; //variable to store user profile details

//initialize wifi and serial port
#define USE_POT_CONTROL 0
#define PRINT_CALLBACK 0

#define PRINT(s, v)         \
    {                       \
        Serial.print(F(s)); \
        Serial.print(v);    \
    }

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define CLK_PIN 18  // or SCK
#define DATA_PIN 23 // or MOSI
#define CS_PIN 5    // or SS

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

#define CHAR_SPACING 1 // pixels between characters
#define BUF_SIZE 75
char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
bool newMessageAvailable = false;

uint16_t scrollDelay = 100; // in milliseconds

void scrollDataSink(uint8_t dev, MD_MAX72XX::transformType_t t, uint8_t col)
// Callback function for data that is being scrolled off the display
{
#if PRINT_CALLBACK
    Serial.print("\n cb ");
    Serial.print(dev);
    Serial.print(' ');
    Serial.print(t);
    Serial.print(' ');
    Serial.println(col);
#endif
}

uint8_t scrollDataSource(uint8_t dev, MD_MAX72XX::transformType_t t)
// Callback function for data that is required for scrolling into the display
{
    static char *p = curMessage;
    static uint8_t state = 0;
    static uint8_t curLen, showLen;
    static uint8_t cBuf[8];
    uint8_t colData;

    // finite state machine to control what we do on the callback
    switch (state)
    {
    case 0: // Load the next character from the font table
        showLen = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state++;

        // if we reached end of message, reset the message pointer
        if (*p == '\0')
        {
            p = curMessage;          // reset the pointer to start of message
            if (newMessageAvailable) // there is a new message waiting
            {
                strcpy(curMessage, newMessage); // copy it in
                newMessageAvailable = false;
            }
        }
        // !! deliberately fall through to next state to start displaying

    case 1: // display the next part of the character
        colData = cBuf[curLen++];
        if (curLen == showLen)
        {
            showLen = CHAR_SPACING;
            curLen = 0;
            state = 2;
        }
        break;

    case 2: // display inter-character spacing (blank column)
        colData = 0;
        if (curLen == showLen)
            state = 0;
        curLen++;
        break;

    default:
        state = 0;
    }

    return (colData);
}

void scrollText(void)
{
    static uint32_t prevTime = 0;

    // Is it time to scroll the text?
    if (millis() - prevTime >= scrollDelay)
    {
        mx.transform(MD_MAX72XX::TSL); // scroll along - the callback will load all the data
        prevTime = millis();           // starting point for next time
    }
}

void setup()
{

    Serial.begin(115200);
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

    deso.addNodePath("https://bitclout.com", bitclout_caRootCert);
    deso.selectDefaultNode(0);

    mx.begin();
    mx.control(MD_MAX72XX::INTENSITY, 1);
    mx.setShiftDataInCallback(scrollDataSource);
    mx.setShiftDataOutCallback(scrollDataSink);

    strcpy(curMessage, " DESO Dashboard ");
    strcpy(newMessage, curMessage);
    newMessageAvailable = true;
}

void loop()
{
    static unsigned long timer1 = 30000UL;
    if (millis() - timer1 > 30000UL)
    {
        
        if (WiFi.isConnected())
        {
            Serial.println("Refreshing rates..");
            deso.updateExchangeRates();

            Serial.print("DeSo Coin Value: $");
            double deso_price=deso.USDCentsPerBitCloutExchangeRate / 100.0;
            Serial.println(deso_price);
            Serial.print("BTC: $");
            Serial.println(deso.USDCentsPerBitcoinExchangeRate / 100.0);
            sprintf(newMessage,"DESO: $%.2f    ",deso_price);
            newMessageAvailable = true;
        }
        timer1 = millis();
    }

    scrollText();
}
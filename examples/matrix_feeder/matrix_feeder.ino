#include <Arduino.h>
#include <MD_MAX72xx.h> //  https://github.com/MajicDesigns/MD_MAX72XX/
#include <SPI.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
DeSoLib deso;
const char ssid[] = "";
const char wifi_pass[] = "";
const char username[] = "";
uint16_t scrollDelay = 50; // in milliseconds

DeSoLib::Profile profile1; // variable to store user profile details
int server_index = 0;
// initialize wifi and serial port
#define USE_POT_CONTROL 0
#define PRINT_CALLBACK 0

#define PRINT(s, v)     \
  {                     \
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
#define BUF_SIZE 200
char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
bool newMessageAvailable = false;


bool endOfMessage = false;
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
      endOfMessage = true;
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
void nextServer()
{
  server_index++; // try different nodes
  if (server_index >= deso.getMaxNodes())
    server_index = 0;
}
void setup()
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

  deso.addNodePath("https://desocialworld.com", GTS_Root_R1_caRootCert);
  deso.addNodePath("https://diamondapp.com", Baltimore_Root_caRootCert);
  deso.addNodePath("https://node.deso.org", GTS_Root_R1_caRootCert);
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

  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 1);
  mx.setShiftDataInCallback(scrollDataSource);
  mx.setShiftDataOutCallback(scrollDataSink);

  strcpy(curMessage, " DESO Feed ");
  strcpy(newMessage, curMessage);
  newMessageAvailable = true;
}
void filter(char *body, char filter_ch)
{
  char *ret;
  do
  {
    ret = strchr(body, filter_ch);
    if (ret == NULL)
      break;
    while (*ret != ' ')
    {
      *ret = 24;
      ret++;
      if (*ret == '\0')
        break;
    }
  } while (ret != NULL);
}

void filter_links(char *body)
{
  char *ret;
  ret = strstr(body, "http");
  if (ret != NULL)
  {
    Serial.println("Found link");

    while (*ret != ' ')
    {
      *ret = 24;
      ret++;
      if (*ret == '\0')
        break;
    }
  }
}

void loop()
{
  if (WiFi.isConnected())
  {

    Serial.println("=== Posts Feed ===");
    int status = deso.updatePostsStatelessSave("", profile1.PublicKeyBase58Check, true, 5, false, 1);
    if (status == 0)
    {
      nextServer();
    }
    Serial.println("\n=== Posts End ===");

    for (int i = 0; i < deso.feeds.size(); i++)
    {
      Serial.printf("[%s]%s\n", deso.feeds[i].username, deso.feeds[i].body);
      snprintf(newMessage, sizeof(newMessage), "[%s] %s ", deso.feeds[i].username, deso.feeds[i].body);
      newMessageAvailable = true;

      while (!endOfMessage)
      {
        scrollText();
      }
      endOfMessage = false;
    }
  }
}

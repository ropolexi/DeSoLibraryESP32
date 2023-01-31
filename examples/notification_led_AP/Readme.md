# DeSo diamond buzzer

## Introduction
This is to demonstrate the use of deso library. 

## Features
- This device will check latest post of a user and give out a buzzer sound when someone gives diamonds. When changing the username and if the diamond count is less than previos result the buzzer will not activate.
- Settings can be changed via web interface at http://esp32deso.local/ 
- Settings will be saved inside the chip.
- An indicator will light when there is a balance change. 
- An indicate will show how many likes and diamonds are there upto 5 from the leds.

## Pre requisite libraries
- https://github.com/bblanchon/ArduinoJson
- https://github.com/ropolexi/DeSo-Arduino-Lib
- https://github.com/squix78/json-streaming-parser


## Components
### ESP32 (devkit v1)

## Settings

Wifi settings, username that needs to be monitored must to be entered in order to this example to work.
Can change the hostname to change the url to access via web interface. If it's esp32deso then url is http://esp32deso.local/


username_default and post_hash_default are default values, after changing the settings via web interface new values will be used even after reboot.

```
#define HOSTNAME "esp32deso"
// Fill in the username
const char username_default[] = "ropolexi";
const char post_hash_default[]= "592e078bf7fceda497577593cbafd52af7a34f08c37435b155169cb07a560da3";
// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";
```

## Demo
https://www.youtube.com/watch?v=i5G4aRErRWo



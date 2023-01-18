# DeSo Post Feed

## Introduction
This is to demostrate the use of deso library. 
Top coin holder has the power to control the relay. Every 10 seconds ESP32 chip check for the top coin holder of the relevant user and give the relay control power to that top coin holder with the use of a specific post like button.

## Pre requisite libraries
- https://github.com/bblanchon/ArduinoJson
- https://github.com/ropolexi/DeSo-Arduino-Lib
- https://github.com/squix78/json-streaming-parser


## Components
### ESP32 (devkit v1)

## Settings

Wifi settings,post and reader public key data need to be entered in order to this example to work

```
// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";
const char username[] = "ropolexi";
const char postHash[] = "a6039c9a223d5fad11b05d54a152d0f77048a1552e06e7430697848db60abf71";
```

## Demo
https://www.youtube.com/watch?v=fsEoGumPW5M



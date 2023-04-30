# DeSo Price Ticker with LED Matrix Display

## Introduction
This is to demonstrate the use of deso library to display information about deso price on a LED matrix display with esp32 microcontroller.
Every 30 seconds it updates the price.

## Pre requisite libraries
- https://github.com/bblanchon/ArduinoJson
- https://github.com/ropolexi/DeSo-Arduino-Lib
- https://github.com/squix78/json-streaming-parser
- https://github.com/MajicDesigns/MD_MAX72XX

## Settings

Wifi settings at line 10 must to be entered in order to this example to work

```
// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";
```

## Demo Video
https://www.youtube.com/watch?v=SxkemuOysz4
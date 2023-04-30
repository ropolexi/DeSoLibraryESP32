# DeSo Post Feeder With LED Matrix Display

## Introduction
This is to demonstrate the use of deso library to display information about following users posts on LED matrix displays with esp32 microcontroller.

## Pre requisite libraries
- https://github.com/bblanchon/ArduinoJson
- https://github.com/ropolexi/DeSo-Arduino-Lib
- https://github.com/squix78/json-streaming-parser
- https://github.com/MajicDesigns/MD_MAX72XX

## Settings

Wifi settings and username must to be entered in order to this example to work, scroll speed can be adjusted with scrollDelay.

```
const char ssid[] = "";
const char wifi_pass[] = "";
const char username[] = "";
uint16_t scrollDelay = 100; // in milliseconds
```

## Demo Video
https://www.youtube.com/watch?v=U9zAxOEaVvg
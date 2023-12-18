# DeSo Post Feed

## Introduction
This is to demostrate the use of deso library to display a post information with esp32 microcontroller.
 
When a specific reader react to a specific post with a like reaction the led will switch on (currently implemented on diamondapp).
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

const char userPublicKey[] = "BC1YLhBH8oPqRRAejbUu9BbFquhLFz6GQ8ZbKk6SAWtFsbdcthVTEw8";
const char postHash[]="a6039c9a223d5fad11b05d54a152d0f77048a1552e06e7430697848db60abf71";
```

## Demo




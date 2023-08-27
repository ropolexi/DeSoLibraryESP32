# DeSo Arduino Library

## Introduction
This is an experimental personal arduino library to monitor the DeSo (Decentralized Social Network).

"DeSo is a new layer-1 blockchain built from the ground up to scale decentralized social applications to one billion users" [https://www.deso.org]

## Features
- Multiple DeSo Nodes supported for decentralization 
- DeSo Coin Value
- Creator Coin Price using username or PublicKey
- All HODLE Asset balance
- Wallet Balance
- Top 10 Hodlers
- Last post like count
- Last post diamond count
- Global Recent posts feed
- Actual hodle asset sum 
- Count Post Reaction Association

## Device Supported

![esp32](esp32.jpg)

ESP32 Module

   
## Changes
- (2021-10-1) support old and new api changes due to rebranding.
- (2022-12-3) 
  - calculate actual hodle assets value using bonding curve equation
  - avoid using updateUsersStateless due to high memory demand for that api when SkipHodlings is false
  - use getUserBalance to get wallet balance and updateHodleAssetBalance to get hodle assets actual value
- (2022-12-4) Hodling asset balance for all the assets by retrieving 10 assets at a time to avoid memory overflow
- (2022-12-5) wallet balance json decode using json-streaming-parser to avoid long list of UTXOs (unpend transactions). Now faster decoding for wallet balance.
- (2023-1-30) Updated functions updateTopHolders, updateHodleAssetBalance,updateSinglePost,updateSingleProfile, updatePostsStateless and updateLastNumPostsForPublicKey to support large json data 
- (2023-8-20) Post reaction association support
## Dependency Libraries
- ArduinoJson - https://github.com/bblanchon/ArduinoJson

**DeSoLibraryESP32 library does not need any seed phrase to access any account. This library is for monitoring and experimental purposes only.**
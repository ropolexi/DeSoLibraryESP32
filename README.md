# DeSo Arduino Library

## Introduction
This is an unofficial personal arduino library for the DeSo (Decentralized Social) Blockchain.

"DeSo is a new layer-1 blockchain built from the ground up to scale decentralized social applications to one billion users" [https://www.deso.org]

## Features
- Multiple DeSo Nodes supported for decentralization 
- DeSo Coin Value
- Creator Coin Price using username or PublicKey
- Wallet Balance (V0 API)
- All HODLE Asset balance
- Wallet Balance
- Top 10 Hodlers
- Last post like count
- Last post diamond count
- V1 API to get balance
- Global Recent posts feed
- Actual hodle asset sum 

Note: Top 10 Hodlers (Recommand Top 5 in case memory issue)

## Serial Output Results
```
DeSo Node: https://bitclout.com
Node Status: Synced OK
DeSo Coin Value: $121.90
=======Profile========
Username: ropolexi
PublicKey: BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8
Creator Coin Price: $31.68
Wallet Balance: $1.02
Total HODLE assets : 4
Total HODLE Asset Balance: $35.40
======================

DeSo Node: https://nachoaverage.com
Node Status: Synced OK
DeSo Coin Value: $121.90
=======Profile========
Username: ropolexi
PublicKey: BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8
Creator Coin Price: $31.68
Wallet Balance: $1.02
Total HODLE assets : 4
Total HODLE Asset Balance: $35.40
======================

DeSo Node: https://members.giftclout.com
Node Status: Synced OK
DeSo Coin Value: $121.90
=======Profile========
Username: ropolexi
PublicKey: BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8
Creator Coin Price: $31.68
Wallet Balance: $1.02
Total HODLE assets : 4
Total HODLE Asset Balance: $35.40
======================
```
## Device Supported

![esp32](esp32.jpg)

ESP32 Module



## Functions
- void addNodePath(const char* url,const char* cert);
- int getMaxNodes();
- void selectDefaultNode(int index);
- char* getSelectedNodeUrl();
- bool getSelectedNodeStatus();
- int updateNodeHealthCheck();
- int updateExchangeRates();
- int updateSingleProfile(const char *username,const char *PublicKeyBase58Check,Profile *prof);
- int updateUsersStateless(const char *PublicKeysBase58Check,bool SkipHodlings,Profile *prof);
- int updateHodlersForPublicKey(const char *username,const char *PublicKeyBase58Check,int NumToFetch,Profile *prof);
- void clearTopHodlersUserNames(Profile *prof);
- int updateLastPostForPublicKey(const char *PublicKeysBase58Check,Profile *prof);
- int updateLastNumPostsForPublicKey(const char *PublicKeysBase58Check,int NumToFetch,Profile *prof);
- int updateUsersBalance(const char *PublicKeysBase58Check,Profile *prof);
- const char *getPostsStateless(const char *messagePayload);
- char *genLocaltime(time_t ts);
- int updatePostsStateless(const char *postHashHex,const char *readerPublicKeyBase58Check,int numToFetch,bool getPostsForGlobalWhitelist,long timePeriod);
- int updateHodlersValuesForPublicKey(const char *username, const char *PublicKeyBase58Check, Profile *prof);
## Changes
- (2021-10-1) support old and new api changes due to rebranding.
- (2022-12-3) 
  - calculate actual hodle assets value using bonding curve equation
  - avoid using updateUsersStateless due to high memory demand for that api when SkipHodlings is false
  - use getUserBalance to get wallet balance and updateHodlersValuesForPublicKey to get hodle assets actual value
## Dependency Libraries
ArduinoJson - https://github.com/bblanchon/ArduinoJson

## Limitations
Tested an account with the following parameters
- following 500
- assets (Hodle) 13

No low memory issues. 
Always try to do the project on a fresh account to get optimum results for the end user device such as a vending machine.

But if the number of following is huge (more than 600) and number of assets are huge number(greater than 50) 
account balance or all the assets hodle balance will not be shown due to memory limitation.

**This library does not need any seed phrase to access any accounts**

**Use this library at your own risk, I will not take any responsible for any damages using this library**


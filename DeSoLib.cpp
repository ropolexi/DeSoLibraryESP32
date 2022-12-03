#include "DeSoLib.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include "math.h"
#define DEBUG_LOG false

DeSoLib::DeSoLib()
{
}
void DeSoLib::addNodePath(const char *url, const char *cert)
{
    Node n;
    strncpy(n.url, url, sizeof(n.url));
    n.caRootCert = cert;
    nodePaths.push_back(n);
}
int DeSoLib::getMaxNodes()
{
    return nodePaths.size();
}

void DeSoLib::selectDefaultNode(int index)
{
    selectedNodeIndex = index;
}
char *DeSoLib::getSelectedNodeUrl()
{
    return nodePaths[selectedNodeIndex].url;
}
bool DeSoLib::getSelectedNodeStatus()
{
    return nodePaths[selectedNodeIndex].status;
}

const char *DeSoLib::getRequest(const char *apiPath)
{
    HTTPClient https;
    // char url_str[100];
    const char *buff_ptr;
    memset(buff_large, 0, sizeof(buff_large));
    if (strcmp(nodePaths[selectedNodeIndex].caRootCert, ""))
    {
        espClientSecure.setCACert(nodePaths[selectedNodeIndex].caRootCert);
    }
    else
    {
        Serial.println("Selecting insecure method,not checking site authenticity");
        espClientSecure.setInsecure();
    }
    https.addHeader("User-Agent", "Mozilla/5.0");
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // snprintf(url_str, sizeof(url_str), "%s%s", nodePaths[selectedNodeIndex].url, apiPath);
    snprintf(buff_small_1, sizeof(buff_small_1), "%s%s", nodePaths[selectedNodeIndex].url, apiPath);

    if (https.begin(espClientSecure, buff_small_1))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            if (httpCode == HTTP_CODE_OK)
            {
                strncpy(buff_large, https.getString().c_str(), sizeof(buff_large));
                // buff_ptr = https.getString().c_str();
            }
            else
            {
                debug_print(httpCode);
            }
        }
        else
        {
            debug_print(httpCode);
        }
    }
    else
    {
        debug_print("Error https");
    }
    https.end();
    buff_ptr = buff_large;
    return buff_ptr;
}
const char *DeSoLib::postRequest(const char *apiPath, const char *data)
{
    HTTPClient https;
    static char buff_null[] = "{}";
    const char *buff_ptr;
    buff_ptr = buff_null;
    // memset(buff_large, 0, sizeof(buff_large));
    if (strcmp(nodePaths[selectedNodeIndex].caRootCert, ""))
    {
        espClientSecure.setCACert(nodePaths[selectedNodeIndex].caRootCert);
    }
    else
    {
        Serial.println("Selecting insecure method,not checking site authenticity");
        espClientSecure.setInsecure();
    }
    snprintf(buff_small_1, sizeof(buff_small_1), "%s%s", nodePaths[selectedNodeIndex].url, apiPath);

    if (https.begin(espClientSecure, buff_small_1))
    {
        https.addHeader("User-Agent", "Mozilla/5.0");
        https.addHeader("Accept", "application/json");
        https.addHeader("Content-Type", "application/json");
        int httpCode = https.POST((uint8_t *)data, strlen(data));
        if (httpCode > 0)
        {
            if (httpCode == HTTP_CODE_OK)
            {

                // if (https.getSize() < 80000)
                //{
                buff_ptr = https.getString().c_str();
                if (strcmp(buff_ptr, "") == 0)
                {
                    buff_ptr = buff_null;
                }
                //}
            }
            else
            {
            }
        }
        else
        {
        }
    }
    else
    {
    }
    https.end();

    // return buff_large;
    return buff_ptr;
}

const char *DeSoLib::getNodeHealthCheck()
{
    return getRequest(RoutePathHealthCheck);
}

int DeSoLib::updateNodeHealthCheck()
{
    int status = 0;
    if (strcmp(getNodeHealthCheck(), "200") == 0)
    {
        status = 1;
        nodePaths[selectedNodeIndex].status = true;
    }
    else
    {
        nodePaths[selectedNodeIndex].status = false;
    }
    return status;
}

const char *DeSoLib::getExchangeRates()
{
    return getRequest(ExchangeRateRoute);
}

int DeSoLib::updateExchangeRates()
{
    int status = 0;
    DynamicJsonDocument doc(1024);
    const char *payload = getExchangeRates();
    debug_print(payload);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error)
    {
        USDCentsPerBitCloutExchangeRate = doc["USDCentsPerBitCloutExchangeRate"];
        if (USDCentsPerBitCloutExchangeRate == 0)
        {
            USDCentsPerBitCloutExchangeRate = doc["USDCentsPerDeSoExchangeRate"]; // api changed
        }
        USDCentsPerBitcoinExchangeRate = doc["USDCentsPerBitcoinExchangeRate"];
        status = 1;
    }
    doc.garbageCollect();
    return status;
}
const char *DeSoLib::getSingleProfile(const char *messagePayload)
{
    return postRequest(RoutePathGetSingleProfile, messagePayload);
}

int DeSoLib::updateSingleProfile(const char *username, const char *PublicKeyBase58Check, Profile *prof)
{
    int status = 0;
    static char postData[100];
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    if (strlen(username) > 0)
    {
        doc["Username"] = username;
    }
    if (strlen(PublicKeyBase58Check) > 0)
    {
        doc["PublicKeyBase58Check"] = PublicKeyBase58Check;
    }

    serializeJson(doc, postData);
    doc.clear();
    const char *payload = getSingleProfile(postData);

    DeserializationError error = deserializeJson(doc, payload);
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
        strncpy(prof->Username, username, sizeof(prof->Username));
        strcpy(prof->PublicKeyBase58Check, "NULL");
    }
    if (!error)
    {
        strlcpy(buff_small_2, doc["Profile"]["Username"] | "0", sizeof(buff_small_2));
        if (strcmp(buff_small_2, "0") != 0)
        {
            strncpy(prof->Username, doc["Profile"]["Username"], sizeof(prof->Username));
            prof->CoinPriceBitCloutNanos = doc["Profile"]["CoinPriceBitCloutNanos"];
            if (prof->CoinPriceBitCloutNanos == 0)
            {
                prof->CoinPriceBitCloutNanos = doc["Profile"]["CoinPriceDeSoNanos"];
            }
            prof->CoinsInCirculationNanos = doc["Profile"]["CoinEntry"]["CoinsInCirculationNanos"];
            strcpy(prof->PublicKeyBase58Check, doc["Profile"]["PublicKeyBase58Check"]);
            status = 1;
        }
        else
        {
            strncpy(prof->Username, username, sizeof(prof->Username));
            strcpy(prof->PublicKeyBase58Check, "NULL");
        }
    }
    else
    {
        debug_print("Json Error");
        strncpy(prof->Username, username, sizeof(prof->Username));
        strcpy(prof->PublicKeyBase58Check, "NULL");
    }
    doc.garbageCollect();
    return status;
}

const char *DeSoLib::getUsersStateless(const char *messagePayload)
{
    return postRequest(RoutePathGetUsersStateless, messagePayload);
}
/**
 * Do not use this api method, memory intensive
 * to get hodles value, use updateHodlersValuesForPublicKey
 * to get wallet balance use getUserBalance
 *
 */
int DeSoLib::updateUsersStateless(const char *PublicKeysBase58Check, bool SkipHodlings, Profile *prof)
{
    int status = 0;
    static char messagePayload[200];
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() * 2 - 5000);

    doc["PublicKeysBase58Check"][0] = PublicKeysBase58Check;
    doc["SkipForLeaderBoard"] = SkipHodlings;
    serializeJson(doc, messagePayload);
    doc.clear();
    const char *payload = getUsersStateless(messagePayload);
    DynamicJsonDocument filter(300);
    filter["UserList"][0]["BalanceNanos"] = true;
    if (SkipHodlings == false)
    {
        filter["UserList"][0]["UsersYouHODL"][0]["BalanceNanos"] = true;
        filter["UserList"][0]["UsersYouHODL"][0]["ProfileEntryResponse"]["CoinPriceDeSoNanos"] = true;
    }

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    else
    {
        // serializeJsonPretty(doc, Serial);
        JsonVariant error = doc["UserList"][0]["BalanceNanos"];
        if (!error.isNull())
        {
            status = 1;
            prof->BalanceNanos = doc["UserList"][0]["BalanceNanos"];
            double HODLBalance = 0;
            double perUserHODLBalanceNanos = 0;
            double perUserHODLValue = 0;
            int TotalHodleNum = 0;
            if (SkipHodlings == false)
            {
                JsonArray arr = doc["UserList"][0]["UsersYouHODL"].as<JsonArray>();
                for (JsonVariant value : arr)
                {
                    perUserHODLBalanceNanos = value["BalanceNanos"].as<double>();
                    perUserHODLValue = value["ProfileEntryResponse"]["CoinPriceDeSoNanos"].as<double>();
                    HODLBalance += ((perUserHODLBalanceNanos / 1000000000.0) * perUserHODLValue) / 1000000000.0;
                    TotalHodleNum++;
                }
                prof->TotalHODLBalanceClout = HODLBalance;
                prof->TotalHodleNum = TotalHodleNum;
            }
        }
        if (!error)
        {
        }
        else
        {
            debug_print("\nJson Error,incomplete due to low memory\n");
        }
    }
    doc.garbageCollect();
    return status;
}

const char *DeSoLib::getHodlersForPublicKey(const char *messagePayload)
{
    return postRequest(RoutePathGetHodlersForPublicKey, messagePayload);
}
int DeSoLib::updateHodlersForPublicKey(const char *username, const char *PublicKeyBase58Check, int NumToFetch, Profile *prof)
{
    int status = 0;
    static char postData[100];
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    if (strlen(username) > 0)
    {
        doc["Username"] = username;
        strncpy(prof->Username, username, sizeof(prof->Username));
    }
    if (strlen(PublicKeyBase58Check) > 0)
    {
        doc["PublicKeyBase58Check"] = PublicKeyBase58Check;
    }
    if (NumToFetch > sizeof(prof->TopHodlersUserNames[0]))
    {
        NumToFetch = sizeof(prof->TopHodlersUserNames[0]);
    }
    doc["NumToFetch"] = NumToFetch;

    serializeJson(doc, postData);
    doc.clear();
    const char *payload = getHodlersForPublicKey(postData);
    DynamicJsonDocument filter(500);
    // StaticJsonDocument<200> filter;
    filter["Hodlers"][0]["BalanceNanos"] = true;
    //filter["Hodlers"][0]["ProfileEntryResponse"]["CoinEntry"]["CoinsInCirculationNanos"] = true;
    // for (int i = 0; i < NumToFetch + 1; i++)
    //{
    filter["Hodlers"][0]["ProfileEntryResponse"]["Username"] = true;
    //}

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        JsonArray arr = doc["Hodlers"].as<JsonArray>();
        int count = 0;
        for (JsonVariant value : arr)
        {
            //if (strcmp(value["ProfileEntryResponse"]["Username"].as<char *>(), prof->Username) != 0)
            //{
                double coins = value["BalanceNanos"].as<double>();
                coins /= 1000000000.0;
                double total_supply = prof->CoinsInCirculationNanos;
                total_supply /= 1000000000.0;
                prof->TopHodlersCoins[count] = coins;
                prof->TopHodlersCoinsPerc[count]=coins*100/total_supply;
                strncpy(prof->TopHodlersUserNames[count], value["ProfileEntryResponse"]["Username"].as<char *>(), sizeof(prof->TopHodlersUserNames[count]));
                count++;
                if (count >= 10)
                    break;
           // }
        }
        status = 1;
    }
    else
    {
        debug_print("Json Error");
    }
    doc.garbageCollect();
    return status;
}

void DeSoLib::clearTopHodlersUserNames(Profile *prof)
{
    for (int i = 0; i < sizeof(prof->TopHodlersUserNames[0]); i++)
    {
        strcpy(prof->TopHodlersUserNames[i], "");
    }
}

/**
 * Get user's total holdles value.
 *
 * Get all hodle creator coins and get actual sell values from bonding curve.
 * bonding curve equation 0.003T^2
 * integration gives the actual sell value
 * Integration Equation 0.001T^3
 * After applying integral limits 0.001(T^3-(T-sell_coins)^3)
 *
 * @param username Username.
 * @param PublicKeyBase58Check Publickey
 * @param prof User profile
 * @return status
 */
int DeSoLib::updateHodlersValuesForPublicKey(const char *username, const char *PublicKeyBase58Check, Profile *prof)
{
    int status = 0;
    static char postData[200];
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    if (strlen(username) > 0)
    {
        doc["Username"] = username;
        strncpy(prof->Username, username, sizeof(prof->Username));
    }
    if (strlen(PublicKeyBase58Check) > 0)
    {
        doc["PublicKeyBase58Check"] = PublicKeyBase58Check;
    }
    doc["NumToFetch"] = 1;
    doc["FetchAll"] = true;
    doc["FetchHodlings"] = true;

    serializeJson(doc, postData);

    doc.clear();
    const char *payload = getHodlersForPublicKey(postData);
    DynamicJsonDocument filter(300);
    filter["Hodlers"][0]["BalanceNanos"] = true;
    filter["Hodlers"][0]["ProfileEntryResponse"]["CoinPriceDeSoNanos"] = true;
    filter["Hodlers"][0]["ProfileEntryResponse"]["CoinEntry"]["CoinsInCirculationNanos"] = true;

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        JsonArray arr = doc["Hodlers"].as<JsonArray>();
        int count = 0;
        double amount = 0;

        for (JsonVariant value : arr)
        {
            double bal = value["BalanceNanos"].as<double>();
            bal /= 1000000000.0;
            double total_coins = value["ProfileEntryResponse"]["CoinEntry"]["CoinsInCirculationNanos"].as<double>();
            total_coins /= 1000000000.0;
            double final_deso_value = pow(total_coins, bonding_curve_pow + 1) - pow((total_coins - bal), bonding_curve_pow + 1);
            final_deso_value *= bonding_curve_gain / 3.0;
            amount += final_deso_value;
            count++;
        }
        prof->TotalHODLBalanceClout = amount;
        prof->TotalHodleNum = count;
        status = 1;
    }
    else
    {
        debug_print("Json Error");
    }
    doc.garbageCollect();
    return status;
}
const char *DeSoLib::getPostsForPublicKey(const char *messagePayload)
{
    return postRequest(RoutePathGetPostsForPublicKey, messagePayload);
}

int DeSoLib::updateLastPostForPublicKey(const char *PublicKeysBase58Check, Profile *prof)
{
    int status = 0;
    static char postData[100];
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    doc["PublicKeyBase58Check"] = PublicKeysBase58Check;
    doc["NumToFetch"] = 1;
    serializeJson(doc, postData);
    doc.clear();
    const char *payload = getPostsForPublicKey(postData);
    DynamicJsonDocument filter(200);
    filter["Posts"][0]["LikeCount"] = true;
    filter["Posts"][0]["DiamondCount"] = true;

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        prof->lastPostLikes = doc["Posts"][0]["LikeCount"];
        prof->lastPostDiamonds = doc["Posts"][0]["DiamondCount"];
        status = 1;
    }
    else
    {
        debug_print("Json Error");
    }
    doc.garbageCollect();
    return status;
}

int DeSoLib::updateLastNumPostsForPublicKey(const char *PublicKeysBase58Check, int NumToFetch, Profile *prof)
{
    int status = 0;
    static char postData[100];
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    doc["PublicKeyBase58Check"] = PublicKeysBase58Check;
    doc["NumToFetch"] = NumToFetch;
    serializeJson(doc, postData);
    doc.clear();
    const char *payload = getPostsForPublicKey(postData);
    DynamicJsonDocument filter(200);
    filter["Posts"][0]["LikeCount"] = true;
    filter["Posts"][0]["DiamondCount"] = true;

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        prof->lastNPostLikes = 0;
        prof->lastNPostDiamonds = 0;

        JsonArray arr = doc["Posts"].as<JsonArray>();
        for (JsonVariant value : arr)
        {
            int likes = value["LikeCount"];
            int diamonds = value["DiamondCount"];
            prof->lastNPostLikes = prof->lastNPostLikes + likes;
            prof->lastNPostDiamonds = prof->lastNPostDiamonds + diamonds;
        }
        status = 1;
    }
    else
    {
        debug_print("Json Error");
    }
    doc.garbageCollect();
    return status;
}

const char *DeSoLib::getUserBalance(const char *messagePayload)
{
    return postRequest(RoutePathGetBalance, messagePayload);
}
int DeSoLib::updateUsersBalance(const char *PublicKeysBase58Check, Profile *prof)
{
    int status = 0;
    static char postData[100];
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    doc["PublicKeyBase58Check"] = PublicKeysBase58Check;
    serializeJson(doc, postData);
    doc.clear();
    const char *payload = getUserBalance(postData);
    DynamicJsonDocument filter(100);
    filter["ConfirmedBalanceNanos"] = true;
    filter["UnconfirmedBalanceNanos"] = true;

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        prof->BalanceNanos = doc["ConfirmedBalanceNanos"];
        prof->UnconfirmedBalanceNanos = doc["UnconfirmedBalanceNanos"];
        status = 1;
    }
    else
    {
        debug_print("Json Error");
    }
    doc.garbageCollect();
    return status;
}

const char *DeSoLib::getPostsStateless(const char *messagePayload)
{
    return postRequest(RoutePathGetPostsStateless, messagePayload);
}

char *DeSoLib::genLocaltime(time_t ts)
{
    struct tm *info;
    info = localtime(&ts);
    char *ts_str = asctime(info);
    ts_str[strlen(ts_str) - 1] = '\0';
    return ts_str;
}

int DeSoLib::updatePostsStateless(const char *postHashHex, const char *readerPublicKeyBase58Check, int numToFetch, bool getPostsForGlobalWhitelist, long timePeriod)
{
    int status = 0;
    char postData[1024];
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    if (strlen(postHashHex) > 0)
    {
        Serial.println("Has postHashHex");
        doc["PostHashHex"] = postHashHex;
    }
    doc["ReaderPublicKeyBase58Check"] = readerPublicKeyBase58Check;
    doc["OrderBy"] = "";
    doc["PostContent"] = "";
    doc["FetchSubcomments"] = false;
    doc["GetPostsForFollowFeed"] = false;
    doc["GetPostsByDESO"] = false;
    doc["NumToFetch"] = numToFetch;
    doc["MediaRequired"] = false;
    doc["PostsByDESOMinutesLookback"] = 0;

    doc["GetPostsForGlobalWhitelist"] = getPostsForGlobalWhitelist;
    serializeJson(doc, postData);
    doc.clear();
    const char *payload = getPostsStateless(postData);
    DynamicJsonDocument filter(100);
    filter["PostsFound"][0]["Body"] = true;
    filter["PostsFound"][0]["TimestampNanos"] = true;
    filter["PostsFound"][0]["ProfileEntryResponse"]["Username"] = true;

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {

        JsonArray arr = doc["PostsFound"].as<JsonArray>();
        for (JsonVariant value : arr)
        {
            String body = value["Body"];
            String timestamp = value["TimestampNanos"];
            String username = value["ProfileEntryResponse"]["Username"];
            if (body.length() > 1)
            {
                time_t ts = strtol(timestamp.substring(0, 10).c_str(), NULL, 10);

                if (time(nullptr) - ts < timePeriod && time(nullptr) - ts > 0)
                {
                    Serial.printf("\n[%s](%s) %s\n", genLocaltime(ts), username.c_str(), body.c_str());
                }
            }
        }

        status = 1;
    }
    else
    {
        debug_print("Json Error");
    }
    doc.garbageCollect();
    return status;
}

DeSoLib::~DeSoLib()
{
}
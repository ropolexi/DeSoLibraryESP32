#include "DeSoLib.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include "math.h"
#include "JsonStreamingParser.h"
#include "JsonListener.h"
#include "Parser.h"

#define DEBUG_LOG true

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

void DeSoLib::addFeed(const char *username, const char *body)
{
    Feed feed;
    strncpy(feed.username, username, sizeof(feed.username));
    strncpy(feed.body, body, sizeof(feed.body));
    feeds.push_back(feed);
}

void DeSoLib::getFeed(int index, char *username, char *body)
{
    strncpy(username, feeds[index].username, sizeof(feeds[index].username));
    strncpy(body, feeds[index].body, sizeof(feeds[index].body));
}

const char *DeSoLib::getRequest(const char *apiPath)
{
    HTTPClient https;
    // char url_str[100];
    const char *buff_ptr;
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
                strncpy(buff_response, https.getString().c_str(), MAX_RESPONSE_SIZE);
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
    buff_ptr = buff_response;
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
                https.getString().toCharArray(buff_response, MAX_RESPONSE_SIZE);
                buff_ptr = buff_response;
                if (strcmp(buff_ptr, "") == 0)
                {
                    buff_ptr = buff_null;
                }
                //}
            }
            else
            {
                debug_print(httpCode);
            }
        }
        else
        {
            debug_print("http error ");
        }
    }
    else
    {
        debug_print("server error ");
    }
    // Serial.printf("https size %d\n", https.getSize());
    https.end();

    // return buff_large;
    return buff_ptr;
}

HTTPClient *DeSoLib::postRequestNew(const char *apiPath, const char *data)
{
    static HTTPClient https;
    https.useHTTP10(true);
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
                return &https;
            }
            else
            {
                debug_print(httpCode);
            }
        }
        else
        {
            debug_print("http error ");
        }
    }
    else
    {
        debug_print("server error ");
    }

    return NULL;
}

const char *DeSoLib::getNodeHealthCheck()
{
    return getRequest(RoutePathHealthCheck);
}

int DeSoLib::updateNodeHealthCheck()
{
    int status = 0;
    buff_response = (char *)malloc(MAX_RESPONSE_SIZE);
    if (strcmp(getNodeHealthCheck(), "200") == 0)
    {
        status = 1;
        nodePaths[selectedNodeIndex].status = true;
    }
    else
    {
        nodePaths[selectedNodeIndex].status = false;
    }
    free(buff_response);
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
    buff_response = (char *)malloc(MAX_RESPONSE_SIZE);
    const char *payload = getExchangeRates();
    // debug_print(payload);
    DeserializationError error = deserializeJson(doc, payload);
    free(buff_response);
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
    HTTPClient *https = postRequestNew(RoutePathGetSingleProfile, postData);
    if (https == NULL)
        return 0;
    DynamicJsonDocument filter(300);
    filter["Profile"]["Username"] = true;
    filter["Profile"]["CoinPriceBitCloutNanos"] = true;
    filter["Profile"]["CoinPriceDeSoNanos"] = true;
    filter["Profile"]["CoinEntry"]["CoinsInCirculationNanos"] = true;
    filter["Profile"]["PublicKeyBase58Check"] = true;

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, https->getStream(), DeserializationOption::Filter(filter));
    https->end();

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

const char *DeSoLib::getHodlersForPublicKey(const char *messagePayload)
{
    return postRequest(RoutePathGetHodlersForPublicKey, messagePayload);
}

void DeSoLib::clearTopHodlersUserNames(Profile *prof)
{
    for (int i = 0; i < sizeof(prof->TopHodlersUserNames[0]); i++)
    {
        strcpy(prof->TopHodlersUserNames[i], "");
    }
}

const char *DeSoLib::getPostsForPublicKey(const char *messagePayload)
{
    return postRequest(RoutePathGetPostsForPublicKey, messagePayload);
}

const char *DeSoLib::getSinglePost(const char *messagePayload)
{
    return postRequest(RoutePathGetSinglePost, messagePayload);
}

int DeSoLib::updateSinglePost(const char *postHashHex, bool fetchParents, int commentOffset, int commentLimit, const char *readerPublicKeyBase58Check, bool addGlobalFeedBool, Post *post)
{
    int status = 0;
    static char postData[1024];
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    strlen(postHashHex) > 0 ? doc["PostHashHex"] = postHashHex : doc["PostHashHex"] = "";
    doc["FetchParents"] = fetchParents;
    doc["CommentOffset"] = commentOffset;
    doc["CommentLimit"] = commentLimit;
    strlen(readerPublicKeyBase58Check) > 0 ? doc["ReaderPublicKeyBase58Check"] = readerPublicKeyBase58Check : doc["ReaderPublicKeyBase58Check"] = "";
    doc["AddGlobalFeedBool"] = addGlobalFeedBool;
    serializeJson(doc, postData);
    doc.clear();
    HTTPClient *https = postRequestNew(RoutePathGetSinglePost, postData);
    if (https == NULL)
        return 0;
    // Serial.println(payload);
    DynamicJsonDocument filter(200);
    filter["PostFound"]["PostHashHex"] = true;
    filter["PostFound"]["Body"] = true;
    filter["PostFound"]["LikeCount"] = true;
    filter["PostFound"]["DiamondCount"] = true;
    filter["PostFound"]["RepostCount"] = true;
    filter["PostFound"]["QuoteRepostCount"] = true;
    filter["PostFound"]["PostEntryReaderState"]["LikedByReader"] = true;

    DeserializationError error = deserializeJson(doc, https->getStream(), DeserializationOption::Filter(filter));
    https->end();
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        strncpy(post->PostHashHex, doc["PostFound"]["PostHashHex"].as<const char*>(), sizeof(post->PostHashHex));
        strncpy(post->Body, doc["PostFound"]["Body"].as<const char*>(), sizeof(post->Body));
        post->LikeCount = doc["PostFound"]["LikeCount"].as<int>();
        post->DiamondCount = doc["PostFound"]["DiamondCount"].as<int>();
        post->RepostCount = doc["PostFound"]["RepostCount"].as<int>();
        post->QuoteRepostCount = doc["PostFound"]["QuoteRepostCount"].as<int>();
        post->LikedByReader = doc["PostFound"]["PostEntryReaderState"]["LikedByReader"].as<bool>();
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
    long mem = ESP.getMaxAllocHeap() / 2 - 5000;
    // Serial.printf("Free Memory:%ld Bytes\n",mem);
    DynamicJsonDocument doc(mem);
    doc["PublicKeyBase58Check"] = PublicKeysBase58Check;
    doc["NumToFetch"] = NumToFetch;
    serializeJson(doc, postData);
    doc.clear();
    HTTPClient *https = postRequestNew(RoutePathGetPostsForPublicKey, postData);
    if (https == NULL)
        return 0;
    DynamicJsonDocument filter(200);
    filter["Posts"][0]["Body"] = true;
    filter["Posts"][0]["LikeCount"] = true;
    filter["Posts"][0]["DiamondCount"] = true;

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, https->getStream(), DeserializationOption::Filter(filter));
    https->end();
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
            String body = value["Body"];
            // Serial.println(value["Body"].as<const char*>());
            if (body.length() > 0) // check without reposts
            {
                int likes = value["LikeCount"];
                int diamonds = value["DiamondCount"];
                prof->lastNPostLikes = prof->lastNPostLikes + likes;
                prof->lastNPostDiamonds = prof->lastNPostDiamonds + diamonds;
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
    buff_response = (char *)malloc(MAX_RESPONSE_SIZE);
    const char *payload = getUserBalance(postData);

    // Using JsonStreamingParser library to extract only needed data and to avoid UTXOs
    JsonStreamingParser parser;
    Listener listener;
    parser.setListener(&listener);
    String key;
    String value;
    for (int i = 0; i < strlen(payload); i++)
    {
        listener.keyFound = false;
        listener.valueFound = false;
        parser.parse(payload[i]);

        if (listener.keyFound)
        {
            if (listener._key.equals("UTXOs"))
                break;
            key = listener._key;
        }
        else if (listener.valueFound)
        {
            if (key.equals("ConfirmedBalanceNanos"))
                prof->BalanceNanos = listener._value.toDouble();
            else if (key.equals("UnconfirmedBalanceNanos"))
                prof->UnconfirmedBalanceNanos = listener._value.toDouble();
        }
    }
    free(buff_response);

    // prof->BalanceNanos = listener.ConfirmedBalanceNanos;
    // prof->UnconfirmedBalanceNanos = listener.UnconfirmedBalanceNanos;
    status = 1;

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
    HTTPClient *https = postRequestNew(RoutePathGetPostsStateless, postData);
    if (https == NULL)
        return 0;
    DynamicJsonDocument filter(100);
    filter["PostsFound"][0]["Body"] = true;
    filter["PostsFound"][0]["TimestampNanos"] = true;
    filter["PostsFound"][0]["ProfileEntryResponse"]["Username"] = true;

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, https->getStream(), DeserializationOption::Filter(filter));
    https->end();
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

int DeSoLib::updatePostsStatelessSave(const char *postHashHex, const char *readerPublicKeyBase58Check, bool getPostsForFollowFeed, int numToFetch, bool getPostsForGlobalWhitelist, int postsByDESOMinutesLookback)
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
    doc["GetPostsForFollowFeed"] = getPostsForFollowFeed;
    doc["GetPostsByDESO"] = true;
    doc["NumToFetch"] = numToFetch;
    doc["MediaRequired"] = false;
    doc["PostsByDESOMinutesLookback"] = postsByDESOMinutesLookback;

    doc["GetPostsForGlobalWhitelist"] = getPostsForGlobalWhitelist;
    serializeJson(doc, postData);
    
    doc.clear();
    HTTPClient *https = postRequestNew(RoutePathGetPostsStateless, postData);
    if (https == NULL)
        return 0;
    DynamicJsonDocument filter(100);
    filter["PostsFound"][0]["Body"] = true;
    filter["PostsFound"][0]["TimestampNanos"] = true;
    filter["PostsFound"][0]["ProfileEntryResponse"]["Username"] = true;

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, https->getStream(), DeserializationOption::Filter(filter));
    https->end();
    //serializeJson(doc, Serial);
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        JsonArray arr = doc["PostsFound"].as<JsonArray>();
        for (JsonVariant value : arr)
        {
            char body[200];
            char username[17];
            strncpy(username, value["ProfileEntryResponse"]["Username"].as<const char*>(), sizeof(username)-1);
            strncpy(body, value["Body"].as<const char*>(), sizeof(body));
            if (strlen(body) > 1)
            {
                Feed feed_filtered;
                int index = 0;
                for (int i = 0; i < strlen(body); i++)
                {
                    if (body[i] >= 32 && body[i] <= 126)
                    {
                        feed_filtered.body[index] = body[i];
                        index++;
                        if (index >= sizeof(feed_filtered.body)-1)
                        {
                            break;
                        }
                    }
                }
                feed_filtered.body[index] = '\0';
                username[sizeof(username)-1]= '\0';

                addFeed(username, feed_filtered.body);
                if (feeds.size() > numToFetch)
                {
                    feeds.erase(feeds.begin());
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

/**
 *
 * @param PublicKeyBase58Check
 * @param Username
 * @param LastPublicKeyBase58Check
 * @param NumToFetch
 * @param IsDAOCoin
 * @param FetchHodlings
 * @param SortType
 * @param FetchAll
 *
 * SortType - "","coin_balance","wealth"
 */

HTTPClient *DeSoLib::updateHodlersForPublicKey(const char *PublicKeyBase58Check,
                                               const char *Username, const char *LastPublicKeyBase58Check, int NumToFetch,
                                               bool IsDAOCoin, bool FetchHodlings, const char *SortType, bool FetchAll, Profile *prof)
{
    static char postData[500];
    DynamicJsonDocument doc(500);
    if (strlen(PublicKeyBase58Check) > 1)
        doc["PublicKeyBase58Check"] = PublicKeyBase58Check;
    if (strlen(Username) > 1)
        doc["Username"] = Username;
    if (strlen(LastPublicKeyBase58Check) > 1)
        doc["LastPublicKeyBase58Check"] = LastPublicKeyBase58Check;
    doc["NumToFetch"] = NumToFetch;
    doc["IsDAOCoin"] = IsDAOCoin;
    doc["FetchHodlings"] = FetchHodlings;
    if (strlen(SortType) > 1)
        doc["SortType"] = SortType;
    doc["FetchAll"] = FetchAll;
    // serializeJsonPretty(doc, Serial);
    serializeJson(doc, postData);
    // Serial.println(postData);
    doc.clear();
    doc.garbageCollect();
    return postRequestNew(RoutePathGetHodlersForPublicKey, postData);
}

/**
 * Get user's total holdles value.
 * Retrieve 10 at a time to avoid memory issue
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
int DeSoLib::updateHodleAssetBalance(const char *username, const char *PublicKeyBase58Check, Profile *prof)
{
    int status = 0;
    char PreLastPublicKey[60];
    char LastPublicKey[60];
    int count = 0;
    double amount = 0;
    memset(LastPublicKey, 0, sizeof(LastPublicKey));
    DynamicJsonDocument filter(300);
    bool first = true;
    int iterations=0;

    filter["LastPublicKeyBase58Check"] = true;
    filter["Hodlers"][0]["BalanceNanos"] = true;
    filter["Hodlers"][0]["ProfileEntryResponse"]["Username"] = true;
    //filter["Hodlers"][0]["ProfileEntryResponse"]["CoinPriceDeSoNanos"] = true;
    filter["Hodlers"][0]["ProfileEntryResponse"]["CoinEntry"]["CoinsInCirculationNanos"] = true;

    while (first || strlen(LastPublicKey) > 1)
    {
        first = false;
        strcpy(PreLastPublicKey, LastPublicKey);

        HTTPClient *https = updateHodlersForPublicKey(PublicKeyBase58Check, username, LastPublicKey, 10, false, true, "", false, prof);
        if (https == NULL)
            return 0;
        long heap_len = ESP.getMaxAllocHeap() / 2 - 5000;
        // Serial.printf("Free memory:%ld\n",heap_len);
        DynamicJsonDocument doc(heap_len);
        DeserializationError error = deserializeJson(doc, https->getStream(), DeserializationOption::Filter(filter));
        https->end();
        if (doc.isNull())
        {
            serializeJsonPretty(doc, Serial);
        }
        if (!error && !doc.isNull() && doc.containsKey("Hodlers"))
        {
            status = 1;

            strcpy(LastPublicKey, doc["LastPublicKeyBase58Check"]);
            JsonArray arr = doc["Hodlers"].as<JsonArray>();

            for (JsonVariant value : arr)
            {
                double bal = value["BalanceNanos"].as<double>();
                bal /= 1000000000.0;
                double total_coins = value["ProfileEntryResponse"]["CoinEntry"]["CoinsInCirculationNanos"].as<double>();
                total_coins /= 1000000000.0;
                double final_deso_value = pow(total_coins, bonding_curve_pow + 1) - pow((total_coins - bal), bonding_curve_pow + 1);
                final_deso_value *= bonding_curve_gain / 3.0;
#if DEBUG_LOG == true
                Serial.print(value["ProfileEntryResponse"]["Username"].as<const char*>());
                Serial.print(":");
                Serial.print((final_deso_value * USDCentsPerBitCloutExchangeRate) / 100.0);
                Serial.printf("(%f)\n", bal);
#endif
                amount += final_deso_value;
                count++;
            }
        }
        else
        {
            status = 0;
            debug_print("Json Error");
        }
        doc.garbageCollect();
        iterations++;
        if(iterations>MAX_HODLING_ITERATIONS){
            break;
        }
    }
    prof->TotalHODLBalanceClout = amount;
    prof->TotalHodleNum = count;

    return status;
}

int DeSoLib::updateTopHolders(const char *username, const char *PublicKeyBase58Check, int NumToFetch, Profile *prof)
{
    int status = 0;
    if (NumToFetch > sizeof(prof->TopHodlersUserNames[0]))
    {
        NumToFetch = sizeof(prof->TopHodlersUserNames[0]);
    }
    // buff_response = (char*)malloc(MAX_RESPONSE_SIZE);
    HTTPClient *https = updateHodlersForPublicKey(PublicKeyBase58Check, "", "", NumToFetch, false, false, "", false, prof);
    if (https == NULL)
        return 0;
    // Serial.println(payload);
    DynamicJsonDocument filter(300);
    filter["Hodlers"][0]["BalanceNanos"] = true;
    filter["Hodlers"][0]["ProfileEntryResponse"]["Username"] = true;
    filter["Hodlers"][0]["ProfileEntryResponse"]["PublicKeyBase58Check"] = true;
    // Deserialize the document
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    DeserializationError error = deserializeJson(doc, https->getStream(), DeserializationOption::Filter(filter));
    https->end();
    // free(buff_response);
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        JsonArray arr = doc["Hodlers"].as<JsonArray>();
        //clear previous data
        for(int i=0;i<TOP_HOLDER_MAX;i++){
            prof->TopHodlersCoins[i]=0;
            prof->TopHodlersCoinsPerc[i]=0;
            memset(prof->TopHodlersUserNames[i],0,sizeof(prof->TopHodlersUserNames[i]));
            memset(prof->TopHodlersPublicKeyBase58Check[i],0,sizeof(prof->TopHodlersPublicKeyBase58Check[i]));
        }
        int count = 0;

        for (JsonVariant value : arr)
        {
            double coins = value["BalanceNanos"].as<double>();
            coins /= 1000000000.0;
            double total_supply = prof->CoinsInCirculationNanos;
            total_supply /= 1000000000.0;
            prof->TopHodlersCoins[count] = coins;
            prof->TopHodlersCoinsPerc[count] = coins * 100 / total_supply;
            strncpy(prof->TopHodlersUserNames[count], value["ProfileEntryResponse"]["Username"].as<const char*>(), sizeof(prof->TopHodlersUserNames[count]));
            strncpy(prof->TopHodlersPublicKeyBase58Check[count], value["ProfileEntryResponse"]["PublicKeyBase58Check"].as<const char*>(), sizeof(prof->TopHodlersPublicKeyBase58Check[count]));
            count++;
            if (count >= 10)
                break;
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

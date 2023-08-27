#include "DeSoLib.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include "math.h"

DeSoLib::DeSoLib()
{
}
/** @brief HTTP GET request.
 *
 *  @param apiPath API path without the domain name
 *  @return HTTPClient pointer
 */
HTTPClient *DeSoLib::getRequest(const char *apiPath)
{
    static HTTPClient https;
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
    snprintf(buff_small_1, sizeof(buff_small_1), "%s%s", nodePaths[selectedNodeIndex].url, apiPath);

    if (https.begin(espClientSecure, String(buff_small_1)))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            if (httpCode == HTTP_CODE_OK)
                return &https;
            else
                debug_print(httpCode);
        }
        else
            debug_print(httpCode);
    }
    else
        debug_print("Error https");

    return NULL;
}

/** @brief get node health.
 * 
 * This will update the node status in nodePaths[selectedNodeIndex]
 *  @param 
 *  @return status
 */
int DeSoLib::updateNodeHealthCheck()
{
    int status = 0;
    HTTPClient *client = getRequest(RoutePathHealthCheck);
    if (client == NULL)
    {
        nodePaths[selectedNodeIndex].status = false;
        return 0;
    }
    snprintf(buff_small_1, sizeof(buff_small_1), client->getString().c_str());
    if (strcmp(buff_small_1, "200") == 0)
    {
        status = 1;
        nodePaths[selectedNodeIndex].status = true;
    }
    else
    {
        debug_print(buff_small_1);
        nodePaths[selectedNodeIndex].status = false;
    }
    client->end();
    return status;
}

/** @brief Update the exchange rates.
 *
 *  @param 
 *  @return status
 */
int DeSoLib::updateExchangeRates()
{
    int status = 0;
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);

    HTTPClient *client = getRequest(ExchangeRateRoute);
    if (client == NULL)
        return 0;

    DynamicJsonDocument filter(300);
    filter["USDCentsPerBitCloutExchangeRate"] = true;
    filter["USDCentsPerDeSoExchangeRate"] = true;
    filter["USDCentsPerBitcoinExchangeRate"] = true;

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, client->getStream(), DeserializationOption::Filter(filter));
    client->end();

    if (doc.isNull())
        return 0;

    if (!error)
    {
        JsonVariant value;
        value = doc["USDCentsPerBitCloutExchangeRate"];
        if (!value.isNull())
            USDCentsPerBitCloutExchangeRate = value.as<double>();
        if (USDCentsPerBitCloutExchangeRate == 0)
        {
            value = doc["USDCentsPerDeSoExchangeRate"];
            if (!value.isNull())
                USDCentsPerBitCloutExchangeRate = value.as<double>(); // api changed
        }
        value = doc["USDCentsPerBitcoinExchangeRate"];
        if (!value.isNull())
            USDCentsPerBitcoinExchangeRate = value.as<double>();
        status = 1;
    }
    else
    {
#if (DEBUG_LOG == true)
        serializeJsonPretty(doc, Serial);
#endif
    }

    doc.garbageCollect();
    return status;
}

/** @brief HTTP POST request.
 *
 *  @param apiPath API path without the domain name
 *  @param data Post data
 *  @return HTTPClient pointer
 */
HTTPClient *DeSoLib::postRequest(const char *apiPath, const char *data)
{
    static HTTPClient https;
    https.useHTTP10(true);
    if (strcmp(nodePaths[selectedNodeIndex].caRootCert, ""))
        espClientSecure.setCACert(nodePaths[selectedNodeIndex].caRootCert);
    else
        espClientSecure.setInsecure();
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
                debug_print(httpCode);
        }
        else
            debug_print("http error ");
    }
    else
        debug_print("server error ");

    return NULL;
}

/** @brief Node app state.
 *  This will printout app status when DEBUG_LOG=true
 *  @param 
 *  @return status
 */
int DeSoLib::getAppState()
{
    int status = 0;
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    HTTPClient *client = postRequest(RoutePathGetAppState, "{}");
    if (client == NULL)
        return 0;
    DeserializationError error = deserializeJson(doc, client->getStream());
    client->end();
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
        return 0;
    }
    if (!error)
    {
        #if(DEBUG_LOG==true)
        serializeJsonPretty(doc, Serial);
        #endif
        status = 1;
    }
    else
    {
        debug_print("Json Error");
    }

    doc.garbageCollect();
    status = 1;
    return status;
}

/** @brief Add a node path
 *  
 *  @param url Domain name of the node
 *  @param cert Root certificate of the domain
 *  @return 
 */
void DeSoLib::addNodePath(const char *url, const char *cert)
{
    Node n;
    strncpy(n.url, url, sizeof(n.url));
    n.caRootCert = cert;
    nodePaths.push_back(n);
}

/** @brief Get maximum nodes added
 *  
 *  @param 
 *  @return Maximum number of nodes
 */
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

void DeSoLib::addUser(const char *username)
{
    Users user;
    strncpy(user.username, username, sizeof(user.username));
    users.push_back(user);
}
void DeSoLib::eraseUsers()
{
    while (users.size() > 0)
    {
        users.erase(users.begin());
    }
}

void DeSoLib::getFeed(int index, char *username, char *body)
{
    strncpy(username, feeds[index].username, sizeof(feeds[index].username));
    strncpy(body, feeds[index].body, sizeof(feeds[index].body));
}

int DeSoLib::updateSingleProfile(const char *username, const char *PublicKeyBase58Check, Profile *prof)
{
    int status = 0;
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    if (strlen(username) > 0)
    {
        doc["Username"] = username;
    }
    if (strlen(PublicKeyBase58Check) > 0)
    {
        doc["PublicKeyBase58Check"] = PublicKeyBase58Check;
    }

    serializeJson(doc, buff_large);
    doc.clear();
    HTTPClient *https = postRequest(RoutePathGetSingleProfile, buff_large);
    if (https == NULL)
        return 0;
    DynamicJsonDocument filter(300);
    filter["Profile"]["Username"] = true;
    filter["Profile"]["CoinPriceBitCloutNanos"] = true;
    filter["Profile"]["CoinPriceDeSoNanos"] = true;
    filter["Profile"]["CoinEntry"]["CoinsInCirculationNanos"] = true;
    filter["Profile"]["PublicKeyBase58Check"] = true;
    filter["Profile"]["DESOBalanceNanos"]=true;

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
        JsonVariant value = doc["Profile"]["Username"];
        if (!value.isNull())
        {
            snprintf(prof->Username, sizeof(prof->Username), value.as<const char *>());
            prof->CoinPriceBitCloutNanos = doc["Profile"]["CoinPriceBitCloutNanos"];
            if (prof->CoinPriceBitCloutNanos == 0)
                prof->CoinPriceBitCloutNanos = doc["Profile"]["CoinPriceDeSoNanos"];
            prof->CoinsInCirculationNanos = doc["Profile"]["CoinEntry"]["CoinsInCirculationNanos"];
            strcpy(prof->PublicKeyBase58Check, doc["Profile"]["PublicKeyBase58Check"]);
            prof->BalanceNanos=doc["Profile"]["DESOBalanceNanos"].as<double>();
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

void DeSoLib::clearTopHodlersUserNames(Profile *prof)
{
    for (int i = 0; i < sizeof(prof->TopHodlersUserNames[0]); i++)
    {
        strcpy(prof->TopHodlersUserNames[i], "");
    }
}

int DeSoLib::countPostAssociation(const char *transactorPublicKeyBase58Check, const char *postHashHex, ReactionCount *reactionCount)
{
    int status = 0;
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    doc["TransactorPublicKeyBase58Check"] = transactorPublicKeyBase58Check;
    doc["PostHashHex"] = postHashHex;
    doc["AppPublicKeyBase58Check"] = "";
    doc["AssociationType"] = "REACTION";
    doc["AssociationValues"][0] = "LIKE";
    doc["AssociationValues"][1] = "DISLIKE";
    doc["AssociationValues"][2] = "LOVE";
    doc["AssociationValues"][3] = "LAUGH";
    doc["AssociationValues"][4] = "ASTONISHED";
    doc["AssociationValues"][5] = "SAD";
    doc["AssociationValues"][6] = "ANGRY";

    serializeJson(doc, buff_large);
    doc.clear();
    HTTPClient *https = postRequest(RoutePathCountPostAssociations, buff_large);
    if (https == NULL)
        return 0;
    DynamicJsonDocument filter(200);
    filter["Counts"]["LIKE"] = true;
    filter["Counts"]["DISLIKE"] = true;
    filter["Counts"]["LOVE"] = true;
    filter["Counts"]["LAUGH"] = true;
    filter["Counts"]["ASTONISHED"] = true;
    filter["Counts"]["SAD"] = true;
    filter["Counts"]["ANGRY"] = true;
    filter["Total"] = true;

    DeserializationError error = deserializeJson(doc, https->getStream(), DeserializationOption::Filter(filter));
    https->end();
    // serializeJsonPretty(doc, Serial);
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        reactionCount->like = doc["Counts"]["LIKE"].as<byte>();
        reactionCount->dislike = doc["Counts"]["DISLIKE"].as<byte>();
        reactionCount->love = doc["Counts"]["LOVE"].as<byte>();
        reactionCount->laugh = doc["Counts"]["LAUGH"].as<byte>();
        reactionCount->astonished = doc["Counts"]["ASTONISHED"].as<byte>();
        reactionCount->sad = doc["Counts"]["SAD"].as<byte>();
        reactionCount->angry = doc["Counts"]["ANGRY"].as<byte>();
        reactionCount->total = doc["Total"].as<int>();
        status = 1;
    }
    else
    {
        debug_print("Json Error");
    }

    doc.garbageCollect();
    status = 1;
    return status;
}

int DeSoLib::countPostAssociationSingle(const char *transactorPublicKeyBase58Check, const char *postHashHex, const char *associationValue, int *count)
{
    int status = 0;
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    doc["TransactorPublicKeyBase58Check"] = transactorPublicKeyBase58Check;
    doc["PostHashHex"] = postHashHex;
    doc["AppPublicKeyBase58Check"] = "";
    doc["AssociationType"] = "REACTION";
    doc["AssociationValue"] = associationValue;

    serializeJson(doc, buff_large);
    doc.clear();
    HTTPClient *https = postRequest(RoutePathCountPostAssociationsSingle, buff_large);
    if (https == NULL)
        return 0;
    DynamicJsonDocument filter(200);
    filter["Count"] = true;

    DeserializationError error = deserializeJson(doc, https->getStream(), DeserializationOption::Filter(filter));
    https->end();
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        *count = doc["Count"].as<int>();
        status = 1;
    }
    else
    {
        debug_print("Json Error");
    }

    doc.garbageCollect();
    status = 1;
    return status;
}
int DeSoLib::updateSinglePost(const char *postHashHex, bool fetchParents, int commentOffset, int commentLimit, const char *readerPublicKeyBase58Check, bool addGlobalFeedBool, Post *post)
{
    int status = 0;
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    strlen(postHashHex) > 0 ? doc["PostHashHex"] = postHashHex : doc["PostHashHex"] = "";
    doc["FetchParents"] = fetchParents;
    doc["CommentOffset"] = commentOffset;
    doc["CommentLimit"] = commentLimit;
    strlen(readerPublicKeyBase58Check) > 0 ? doc["ReaderPublicKeyBase58Check"] = readerPublicKeyBase58Check : doc["ReaderPublicKeyBase58Check"] = "";
    doc["AddGlobalFeedBool"] = addGlobalFeedBool;
    serializeJson(doc, buff_large);
    doc.clear();
    HTTPClient *https = postRequest(RoutePathGetSinglePost, buff_large);
    if (https == NULL)
        return 0;
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
        strncpy(post->PostHashHex, doc["PostFound"]["PostHashHex"].as<const char *>(), sizeof(post->PostHashHex));
        strncpy(post->Body, doc["PostFound"]["Body"].as<const char *>(), sizeof(post->Body));
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
    long mem = ESP.getMaxAllocHeap() / 2 - 5000;
    DynamicJsonDocument doc(mem);
    doc["PublicKeyBase58Check"] = PublicKeysBase58Check;
    doc["NumToFetch"] = NumToFetch;
    serializeJson(doc, buff_large);
    doc.clear();
    HTTPClient *https = postRequest(RoutePathGetPostsForPublicKey, buff_large);
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
            if (body.length() > 0) // check without reposts
            {
                int likes = value["LikeCount"];
                int diamonds = value["DiamondCount"];
                prof->lastNPostLikes = prof->lastNPostLikes + likes;
                prof->lastNPostDiamonds = prof->lastNPostDiamonds + diamonds;
                addFeed(prof->Username, body.c_str());
                if (feeds.size() > NumToFetch)
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
/*
int DeSoLib::updateUsersBalance(const char *PublicKeysBase58Check, Profile *prof)
{
    int status = 0;

    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    doc["PublicKeyBase58Check"] = PublicKeysBase58Check;
    doc["Confirmations"] = 3;
    serializeJson(doc, buff_large);
    doc.clear();
    HTTPClient *client = postRequest(RoutePathGetBalance, buff_large);
    if (client == NULL)
        return 0;
    JsonStreamingParser parser;
    Listener listener;
    parser.setListener(&listener);
    String key;
    String value;
    while (client->getStream().available() > 0)
    {
        listener.keyFound = false;
        listener.valueFound = false;
        char ch=client->getStream().read();
        Serial.print(ch);
        parser.parse(ch);

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
    client->end();
    free(buff_response);
    status = 1;

    doc.garbageCollect();
    return status;
}
*/
char *DeSoLib::genLocaltime(time_t ts)
{
    struct tm *info;
    info = localtime(&ts);
    char *ts_str = asctime(info);
    ts_str[strlen(ts_str) - 1] = '\0';
    return ts_str;
}
/*
timePeriod in seconds
*/
int DeSoLib::updatePostsStateless(const char *postHashHex, const char *readerPublicKeyBase58Check, int numToFetch, bool getPostsForGlobalWhitelist, long timePeriod)
{
    int status = 0;
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
    doc["GetPostsByDESO"] = true;
    doc["NumToFetch"] = numToFetch;
    doc["MediaRequired"] = false;
    doc["PostsByDESOMinutesLookback"] = timePeriod / 60;

    doc["GetPostsForGlobalWhitelist"] = getPostsForGlobalWhitelist;
    serializeJson(doc, buff_large);
    doc.clear();
    HTTPClient *https = postRequest(RoutePathGetPostsStateless, buff_large);
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
    serializeJson(doc, buff_large);

    doc.clear();
    HTTPClient *https = postRequest(RoutePathGetPostsStateless, buff_large);
    if (https == NULL)
        return 0;
    DynamicJsonDocument filter(100);
    filter["PostsFound"][0]["Body"] = true;
    filter["PostsFound"][0]["TimestampNanos"] = true;
    filter["PostsFound"][0]["ProfileEntryResponse"]["Username"] = true;

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, https->getStream(), DeserializationOption::Filter(filter));
    https->end();
    // serializeJson(doc, Serial);
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
            strncpy(username, value["ProfileEntryResponse"]["Username"].as<const char *>(), sizeof(username) - 1);
            strncpy(body, value["Body"].as<const char *>(), sizeof(body));
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
                        if (index >= sizeof(feed_filtered.body) - 1)
                        {
                            break;
                        }
                    }
                }
                feed_filtered.body[index] = '\0';
                username[sizeof(username) - 1] = '\0';

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
    serializeJson(doc, buff_large);
    // Serial.println(buff_large);
    doc.clear();
    doc.garbageCollect();
    return postRequest(RoutePathGetHodlersForPublicKey, buff_large);
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
int DeSoLib::updateHodleAssetBalance(const char *username, const char *PublicKeyBase58Check, Profile *prof, bool save)
{
    int status = 0;
    char PreLastPublicKey[56];
    char LastPublicKey[56];
    int count = 0;
    double amount = 0;
    memset(LastPublicKey, 0, sizeof(LastPublicKey));
    DynamicJsonDocument filter(300);
    bool first = true;
    int iterations = 0;

    filter["LastPublicKeyBase58Check"] = true;
    filter["Hodlers"][0]["BalanceNanos"] = true;
    filter["Hodlers"][0]["ProfileEntryResponse"]["Username"] = true;
    // filter["Hodlers"][0]["ProfileEntryResponse"]["CoinPriceDeSoNanos"] = true;
    filter["Hodlers"][0]["ProfileEntryResponse"]["CoinEntry"]["CoinsInCirculationNanos"] = true;
    if (save)
        eraseUsers();
    while (first || strlen(LastPublicKey) > 1)
    {
        first = false;
        strcpy(PreLastPublicKey, LastPublicKey);

        HTTPClient *https = updateHodlersForPublicKey(PublicKeyBase58Check, username, LastPublicKey, 20, false, true, "", false, prof);
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
            return 0;
        }
        // serializeJsonPretty(doc, Serial);
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

                char username[20];
                if (save && final_deso_value > 0.0001)
                {
                    // strncpy(username, value["ProfileEntryResponse"]["Username"].as<const char *>(), sizeof(username) - 1);
                    // username[sizeof(username) - 1] = '\0';
                    snprintf(username, sizeof(username), "%s", value["ProfileEntryResponse"]["Username"].as<const char *>());
                    addUser(username);
                }
#if DEBUG_LOG == true
                if (final_deso_value > 0.0001)
                {
                    Serial.print(value["ProfileEntryResponse"]["Username"].as<const char *>());
                    Serial.print(": ");
                    Serial.print((final_deso_value * USDCentsPerBitCloutExchangeRate) / 100.0);
                    Serial.printf(" (%f)\n", bal);
                }
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
        if (iterations > MAX_HODLING_ITERATIONS)
        {
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
        // clear previous data
        for (int i = 0; i < TOP_HOLDER_MAX; i++)
        {
            prof->TopHodlersCoins[i] = 0;
            prof->TopHodlersCoinsPerc[i] = 0;
            memset(prof->TopHodlersUserNames[i], 0, sizeof(prof->TopHodlersUserNames[i]));
            memset(prof->TopHodlersPublicKeyBase58Check[i], 0, sizeof(prof->TopHodlersPublicKeyBase58Check[i]));
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
            if (!value.containsKey("ProfileEntryResponse"))
            {
                value["ProfileEntryResponse"]["Username"] = "NONE";
                value["ProfileEntryResponse"]["PublicKeyBase58Check"] = "NONE";
            }
            strncpy(prof->TopHodlersUserNames[count], value["ProfileEntryResponse"]["Username"].as<const char *>(), sizeof(prof->TopHodlersUserNames[count]));
            strncpy(prof->TopHodlersPublicKeyBase58Check[count], value["ProfileEntryResponse"]["PublicKeyBase58Check"].as<const char *>(), sizeof(prof->TopHodlersPublicKeyBase58Check[count]));
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

int DeSoLib::getNFTEntriesForNFTPost(const char *postHashHex, int serialNumber, char *OwnerPublicKeyBase58Check)
{

    int status = 0;
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    if (strlen(postHashHex) > 0)
    {
        doc["PostHashHex"] = postHashHex;
    }
    serializeJson(doc, buff_large);

    doc.clear();
    HTTPClient *https = postRequest(RoutePathNFTEntriesForNFTPost, buff_large);
    if (https == NULL)
        return 0;
    DynamicJsonDocument filter(100);
    filter["NFTEntryResponses"] = true;

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, https->getStream(), DeserializationOption::Filter(filter));
    https->end();
    // serializeJson(doc, Serial);
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        strncpy(OwnerPublicKeyBase58Check, doc["NFTEntryResponses"][serialNumber - 1]["OwnerPublicKeyBase58Check"].as<const char *>(), 56);

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

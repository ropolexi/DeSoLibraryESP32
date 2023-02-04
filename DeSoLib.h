#ifndef DeSoLib_h
#define DeSoLib_h

#include <HTTPClient.h>
#include "WiFi.h"
#include <stdlib.h>
#include "cert.h"

#define DEBUG_LOG true
#define MAX_RESPONSE_SIZE 30000
#define debug_print(...)               \
    do                                 \
    {                                  \
        if (DEBUG_LOG)                 \
            Serial.println(__VA_ARGS__); \
    } while (0)
class DeSoLib
{
public:
    WiFiClientSecure espClientSecure;
    DeSoLib();
    const char *RoutePathHealthCheck = "/api/v0/health-check";
    const char *ExchangeRateRoute = "/api/v0/get-exchange-rate";
    const char *RoutePathGetSingleProfile = "/api/v0/get-single-profile";
    const char *RoutePathGetUsersStateless = "/api/v0/get-users-stateless";
    const char *RoutePathGetHodlersForPublicKey = "/api/v0/get-hodlers-for-public-key";
    const char *RoutePathGetPostsForPublicKey = "/api/v0/get-posts-for-public-key";
    const char *RoutePathGetSinglePost = "/api/v0/get-single-post";
    const char *RoutePathGetBalance = "/api/v1/balance";
    const char *RoutePathGetPostsStateless = "/api/v0/get-posts-stateless";

    struct Node
    {
        char url[50];
        bool status = false;
        const char *caRootCert;
    };
    struct Post
    {
        char PostHashHex[65];
        char Body[1024];
        int LikeCount=0;
        int DiamondCount=0;
        int CommentCount=0;
        int RepostCount=0;
        int QuoteRepostCount=0;
        bool LikedByReader=false;
    };

    struct Feed
    {
        char username[17];
        char body[180];//for lcd display limit
    };
    struct Profile
    {
        char PublicKeyBase58Check[56];
        char Username[20];
        double CoinsInCirculationNanos = 0;
        double CoinPriceBitCloutNanos = 0;
        double BalanceNanos = 0;
        double UnconfirmedBalanceNanos = 0;
        double TotalHODLBalanceClout = 0;
        int TotalHodleNum = 0;
        char TopHodlersUserNames[10][20];
        char TopHodlersPublicKeyBase58Check[10][56];
        double TopHodlersCoins[10];
        double TopHodlersCoinsPerc[10];
        int lastNPostLikes = 0;
        int lastNPostDiamonds = 0;
    };

    std::vector<Feed> feeds;

    char buff_small_1[200];
    char buff_small_2[200];
    //char buff_large[1024]; // heavy usage on web response
    char *buff_response;
    double USDCentsPerBitCloutExchangeRate;
    double USDCentsPerBitcoinExchangeRate;

    int selectedNodeIndex = 0;
    char *genLocaltime(time_t ts);
    void addNodePath(const char *url, const char *cert);
    int getMaxNodes();
    void selectDefaultNode(int index);
    char *getSelectedNodeUrl();
    bool getSelectedNodeStatus();
    void addFeed(const char *username, const char *body);
    const char *getRequest(const char *apiPath);
    const char *postRequest(const char *apiPath, const char *data);
    const char *getNodeHealthCheck();
    int updateNodeHealthCheck();
    const char *getExchangeRates();
    int updateExchangeRates();
    const char *getSingleProfile(const char *messagePayload);
    int updateSingleProfile(const char *username, const char *PublicKeyBase58Check, Profile *prof);
    const char *getUsersStateless(const char *messagePayload);
    const char *getHodlersForPublicKey(const char *messagePayload);
    int updateHodlersForPublicKey(const char *username, const char *PublicKeyBase58Check, int NumToFetch, Profile *prof);
    void clearTopHodlersUserNames(Profile *prof);
    const char *getSinglePost(const char *messagePayload);
    int updateSinglePost(const char *postHashHex, bool fetchParents, int commentOffset, int commentLimit, const char *readerPublicKeyBase58Check,bool addGlobalFeedBool, Post *post);
    const char *getPostsForPublicKey(const char *messagePayload);
    int updateLastNumPostsForPublicKey(const char *PublicKeysBase58Check, int NumToFetch, Profile *prof);
    const char *getUserBalance(const char *messagePayload);
    int updateUsersBalance(const char *PublicKeysBase58Check, Profile *prof);
    const char *getPostsStateless(const char *messagePayload);
    int updatePostsStateless(const char *postHashHex, const char *readerPublicKeyBase58Check, int numToFetch, bool getPostsForGlobalWhitelist, long timePeriod);
    HTTPClient *updateHodlersForPublicKey(const char *PublicKeyBase58Check,
                                          const char *Username, const char *LastPublicKeyBase58Check, int NumToFetch,
                                          bool IsDAOCoin, bool FetchHodlings, const char *SortType, bool FetchAll, Profile *prof);
    int updateHodleAssetBalance(const char *username, const char *PublicKeyBase58Check, Profile *prof);
    int updateTopHolders(const char *username, const char *PublicKeyBase58Check, int NumToFetch, Profile *prof);
    HTTPClient *postRequestNew(const char *apiPath, const char *data);
    int updatePostsStatelessSave(const char *postHashHex, const char *readerPublicKeyBase58Check,bool getPostsForFollowFeed, int numToFetch, bool getPostsForGlobalWhitelist, int postsByDESOMinutesLookback);
    void getFeed(int index,char *username,char *body);
    //const char *getSinglePost(const char *messagePayload);
    //const char *updateSinglePost(const char *PostHashHex, bool FetchParents, int CommentOffset, int CommentLimit, const char *ReaderPublicKeyBase58Check,Profile *prof);
    //const char *getComments(const char *PostHashHex,int CommentOffset,int CommentLimit);
    ~DeSoLib();

private:
    std::vector<Node> nodePaths;
    
    double bonding_curve_gain = 0.003;
    int bonding_curve_pow = 2;
};
#endif
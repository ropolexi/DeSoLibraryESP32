#ifndef DeSoLib_h
#define DeSoLib_h

#include <HTTPClient.h>
#include "WiFi.h"
#include <stdlib.h>
#include "cert.h"

#define DEBUG_LOG true
#define MAX_RESPONSE_SIZE 30000
#define TOP_HOLDER_MAX 10
//Max iterations , each iteration requests 10 hodling assets
#define MAX_HODLING_ITERATIONS 10
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
    //General Endpoints
    const char *RoutePathHealthCheck = "/api/v0/health-check";
    const char *ExchangeRateRoute = "/api/v0/get-exchange-rate";
    const char *RoutePathGetAppState="/api/v0/get-app-state";

    //User Endpoints
    const char *RoutePathGetUsersStateless = "/api/v0/get-users-stateless";
    const char *RoutePathGetProfiles="/api/v0/get-profiles";
    const char *RoutePathGetSingleProfile = "/api/v0/get-single-profile";

    //Post Endpoints
    const char *RoutePathGetPostsStateless = "/api/v0/get-posts-stateless";
    const char *RoutePathGetSinglePost = "/api/v0/get-single-post";
    const char *RoutePathGetPostsForPublicKey = "/api/v0/get-posts-for-public-key";
    const char *RoutePathGetHotFeed = "/api/v0/get-hot-feed";
    const char *RoutePathGetDiamondedPosts="/api/v0/get-diamonded-posts";
    const char *RoutePathGetLikesForPost="/api/v0/get-likes-for-post";
    const char *RoutePathGetDiamondsForPost="/api/v0/get-diamonds-for-post";
    const char *RoutePathGetRepostsForPost="/api/v0/get-reposts-for-post";
    const char *RoutePathGetQuoteRepostsForPost="/api/v0/get-quote-reposts-for-post";
    
    
    const char *RoutePathGetHodlersForPublicKey = "/api/v0/get-hodlers-for-public-key";
    
    
    const char *RoutePathGetBalance = "/api/v1/balance";
    
    
    const char *CountPostAssociationsSingle = "/api/v0/post-associations/count";
    const char *CountPostAssociations = "/api/v0/post-associations/counts";
    const char *NFTEntriesForNFTPost = "/api/v0/get-nft-entries-for-nft-post";

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

    struct ReactionCount
    {
        byte like = 0;
        byte dislike = 0;
        byte love = 0;
        byte sad = 0;
        byte angry = 0;
        byte astonished = 0;
        byte laugh = 0;
        int total = 0;
    };

    struct Feed
    {
        char username[17];
        char body[180];//for lcd display limit
    };

    struct Users
    {
        char username[20];
    };
    
    struct PostEntryResponse
    {

    };
    struct CoinEntryResponse
    {
        uint64_t CreatorBasisPoints;        
        uint64_t DeSoLockedNanos;           
        uint64_t NumberOfHolders;           
        uint64_t CoinsInCirculationNanos;  
        uint64_t CoinWatermarkNanos;

    };

    struct DAOCoinEntryResponse
    {
        uint64_t NumberOfHolders;
        uint64_t CoinsInCirculationNanos;
        uint64_t MintingDisabled;
        char *TransferRestrictionStatus;
    };

    struct BalanceEntryResponse
    {

    };
    struct ProfileEntryResponse
    {
        char PublicKeyBase58Check[56];
        char Username[20];
        char Description[20];
        bool IsHidden;
        bool IsReserved;
        bool IsVerified;
        PostEntryResponse *Comments;
        PostEntryResponse *postEntryResponse;
        CoinEntryResponse *CoinEntry;
        DAOCoinEntryResponse *DAOCoinEntry;
        uint64_t CoinPriceDeSoNanos;
        BalanceEntryResponse *UsersThatHODL;
        bool IsFeaturedTutorialWellKnownCreator;
        bool IsFeaturedTutorialUpAndComingCreator;
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
        char TopHodlersUserNames[TOP_HOLDER_MAX][20];
        char TopHodlersPublicKeyBase58Check[TOP_HOLDER_MAX][56];
        double TopHodlersCoins[TOP_HOLDER_MAX];
        double TopHodlersCoinsPerc[TOP_HOLDER_MAX];
        int lastNPostLikes = 0;
        int lastNPostDiamonds = 0;
    };

    std::vector<Feed> feeds;
    std::vector<Users> users;
 
    char buff_small_1[200];
    char buff_small_2[200];
    char buff_large[1024];
    char *buff_response;
    double USDCentsPerBitCloutExchangeRate;
    double USDCentsPerBitcoinExchangeRate;

    int selectedNodeIndex = 0;
    HTTPClient *getRequest(const char *apiPath);
    int updateNodeHealthCheck();
    int updateExchangeRates();
    HTTPClient *postRequest(const char *apiPath, const char *data);
    int getAppState();

    char *genLocaltime(time_t ts);
    void addNodePath(const char *url, const char *cert);
    int getMaxNodes();
    void selectDefaultNode(int index);
    char *getSelectedNodeUrl();
    bool getSelectedNodeStatus();
    void addFeed(const char *username, const char *body);
    int updateSingleProfile(const char *username, const char *PublicKeyBase58Check, Profile *prof);
    int updateHodlersForPublicKey(const char *username, const char *PublicKeyBase58Check, int NumToFetch, Profile *prof);
    void clearTopHodlersUserNames(Profile *prof);
    int updateSinglePost(const char *postHashHex, bool fetchParents, int commentOffset, int commentLimit, const char *readerPublicKeyBase58Check,bool addGlobalFeedBool, Post *post);
    int updateLastNumPostsForPublicKey(const char *PublicKeysBase58Check, int NumToFetch, Profile *prof);
    const char *getUserBalance(const char *messagePayload);
    int updateUsersBalance(const char *PublicKeysBase58Check, Profile *prof);
    const char *getPostsStateless(const char *messagePayload);
    int updatePostsStateless(const char *postHashHex, const char *readerPublicKeyBase58Check, int numToFetch, bool getPostsForGlobalWhitelist, long timePeriod);
    HTTPClient *updateHodlersForPublicKey(const char *PublicKeyBase58Check,
                                          const char *Username, const char *LastPublicKeyBase58Check, int NumToFetch,
                                          bool IsDAOCoin, bool FetchHodlings, const char *SortType, bool FetchAll, Profile *prof);
    int updateHodleAssetBalance(const char *username, const char *PublicKeyBase58Check,  Profile *prof,bool save=false);
    int updateTopHolders(const char *username, const char *PublicKeyBase58Check, int NumToFetch, Profile *prof);
    
    int updatePostsStatelessSave(const char *postHashHex, const char *readerPublicKeyBase58Check,bool getPostsForFollowFeed, int numToFetch, bool getPostsForGlobalWhitelist, int postsByDESOMinutesLookback);
    void getFeed(int index,char *username,char *body);
    void addUser(const char *username);
    void eraseUsers();
    int countPostAssociation(const char* transactorPublicKeyBase58Check, const char* postHashHex, ReactionCount* reactionCount);
    int countPostAssociationSingle(const char *transactorPublicKeyBase58Check, const char *postHashHex,const char* associationValue, int* count);
    int getNFTEntriesForNFTPost(const char *postHashHex,int serialNumber,char *OwnerPublicKeyBase58Check);
    ~DeSoLib();

private:
    std::vector<Node> nodePaths;
    
    double bonding_curve_gain = 0.003;
    int bonding_curve_pow = 2;
};
#endif

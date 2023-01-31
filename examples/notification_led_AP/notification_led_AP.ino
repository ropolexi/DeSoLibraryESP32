#include <Arduino.h>
#include <Preferences.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "ArduinoJson.h"

#define LIKE_1_PIN 4
#define LIKE_2_PIN 16
#define LIKE_3_PIN 17
#define LIKE_4_PIN 5
#define LIKE_5_PIN 18

#define DIAMOND_1_PIN 19
#define DIAMOND_2_PIN 21
#define DIAMOND_3_PIN 22
#define DIAMOND_4_PIN 23
#define DIAMOND_5_PIN 32

#define BALANCE_PIN 15
#define BUZZER_PIN 13

#define HOSTNAME "esp32deso"
// Fill in the username
const char username_default[] = "ropolexi";
const char post_hash_default[]= "592e078bf7fceda497577593cbafd52af7a34f08c37435b155169cb07a560da3";
char username[20];
// Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";

WebServer server(80);
DeSoLib deso;
DeSoLib::Profile profile1;
DeSoLib::Post post1;
Preferences preferences;
// DeSoLib::Post post1;
int diamond_count = 0;
int like_count = 0;
int server_index = 0;
double balance = 0;
int post_type = 0;
int num_posts = 0;
char post_hash[65];
bool username_updated=false;
bool force_update=false;
const char main_page[] PROGMEM = R"(
<html>
<title>
    DeSo Dashbaord
</title>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style></style>
</head>
<body bgcolor="#2CA2E1" style="font-family:arial">
    <h1>ESP32 DESO Dashboard Settings</h1>
    <h2>Settings</h2>
    <form method="POST" action="/settings/">
        <table width="100%">
            <col width="20%">
            <col width="80%">
            <tr>
                <td>Username</td>
                <td><input type="text" name="username" id="username" value="" maxlength="20"></td>
            </tr>
            <tr>
                <td>Read Post</td>
                <td>
                    <select class="select" name="post_type" id="post_type">
                        <option value="latest">Latest</option>
                        <option value="fixed">Fixed</option>
                    </select>
            <tr class="num">
                <td>Number of posts</td>
                <td><input type="number" name="num" id="num" value="" maxlength="3"></td>
            </tr>
            <tr class="hash">
                <td>Post hash</td>
                <td><input type="text" name="post_hash" id="post_hash" value="" maxlength="65" size="65"></td>
            </tr>
            </td>
            </tr>
            <tr>
                <td></td>
                <td><input type="submit" value="Submit"></td>
            </tr>
            <tr>
                <td><b>Results will update, Please wait...</b></td>
                <td></td>
            </tr>
            <tr>
                <td>Node</td>
                <td>
                    <div id="node"></div>
                </td>
            </tr>
            <tr>
                <td>Diamonds</td>
                <td>
                    <div id="diamonds"></div>
                </td>
            </tr>
            <tr>
                <td>Likes</td>
                <td>
                    <div id="likes"></div>
                </td>
            </tr>
            <tr>
                <td>Balance (DESO)</td>
                <td>
                    <div id="balance"></div>
                </td>
            </tr>
        </table>
    </form>
    <script>
        const select1 = document.getElementsByClassName("select")[0]
        const num = document.getElementsByClassName("num")[0]
        const hash = document.getElementsByClassName("hash")[0]
        if (select1.options[select1.selectedIndex].value == "latest") {
            num.style.display = "table-row"
            hash.style.display = "none"
        } else if (select1.options[select1.selectedIndex] == "fixed") {
            num.style.display = "none"
            hash.style.display = "table-row"
        }
        select1.addEventListener('change', function handleChange(event) {
            if (event.target.value == "latest") {
                num.style.display = "table-row"
                hash.style.display = "none"
            } else if (event.target.value == "fixed") {
                num.style.display = "none"
                hash.style.display = "table-row"
            }
        })
        function update_data() {
            const node = document.getElementById("node")
            const diamonds = document.getElementById("diamonds")
            const likes = document.getElementById("likes")
            const balance = document.getElementById("balance")
            const xhttp = new XMLHttpRequest();
            xhttp.onload = function () {
                var jsonObj = JSON.parse(this.responseText);
                node.innerHTML = jsonObj.node
                diamonds.innerHTML = jsonObj.diamonds
                likes.innerHTML = jsonObj.likes
                balance.innerHTML = jsonObj.balance
            }
            xhttp.open("GET", "/info/", true);
            xhttp.send();
        }

        function loadDoc() {
            const username_item = document.getElementById("username")
            const type_item = document.getElementById("post_type")
            const num_item = document.getElementById("num")
            const hash_item = document.getElementById("post_hash")
            const xhttp = new XMLHttpRequest();
            xhttp.onload = function () {
                var jsonObj = JSON.parse(this.responseText);
                if (jsonObj.type == 0) {
                    type_item.value = "latest"
                    num.style.display = "table-row"
                    hash.style.display = "none"
                } else if (jsonObj.type == 1) {
                    type_item.value = "fixed"
                    num.style.display = "none"
                    hash.style.display = "table-row"
                }
                num_item.value = jsonObj.num
                hash_item.value = jsonObj.hash
                username_item.value = jsonObj.username
            }
            xhttp.open("GET", "/info/", true);
            xhttp.send();
        }
        loadDoc();
        setInterval(update_data, 2000)
    </script>
</body>
</html>
)";
void nextServer()
{
  server_index++; // try different nodes
  if (server_index >= deso.getMaxNodes())
    server_index = 0;
}

void like_led_test()
{
  digitalWrite(LIKE_1_PIN, HIGH);
  delay(100);
  digitalWrite(LIKE_2_PIN, HIGH);
  delay(100);
  digitalWrite(LIKE_3_PIN, HIGH);
  delay(100);
  digitalWrite(LIKE_4_PIN, HIGH);
  delay(100);
  digitalWrite(LIKE_5_PIN, HIGH);
  delay(100);
}

void diamond_led_clear()
{
  digitalWrite(DIAMOND_1_PIN, LOW);
  digitalWrite(DIAMOND_2_PIN, LOW);
  digitalWrite(DIAMOND_3_PIN, LOW);
  digitalWrite(DIAMOND_4_PIN, LOW);
  digitalWrite(DIAMOND_5_PIN, LOW);
}

void like_led_clear()
{
  digitalWrite(LIKE_1_PIN, LOW);
  digitalWrite(LIKE_2_PIN, LOW);
  digitalWrite(LIKE_3_PIN, LOW);
  digitalWrite(LIKE_4_PIN, LOW);
  digitalWrite(LIKE_5_PIN, LOW);
}
void diamond_led_test()
{
  digitalWrite(DIAMOND_1_PIN, HIGH);
  delay(100);
  digitalWrite(DIAMOND_2_PIN, HIGH);
  delay(100);
  digitalWrite(DIAMOND_3_PIN, HIGH);
  delay(100);
  digitalWrite(DIAMOND_4_PIN, HIGH);
  delay(100);
  digitalWrite(DIAMOND_5_PIN, HIGH);
  delay(100);
}

void diamond_led_indicate(int num)
{
  diamond_led_clear();
  switch (num)
  {
  case 0:
    break;
  case 1:
    digitalWrite(DIAMOND_1_PIN, HIGH);
    break;
  case 2:
    digitalWrite(DIAMOND_1_PIN, HIGH);
    digitalWrite(DIAMOND_2_PIN, HIGH);
    break;
  case 3:
    digitalWrite(DIAMOND_1_PIN, HIGH);
    digitalWrite(DIAMOND_2_PIN, HIGH);
    digitalWrite(DIAMOND_3_PIN, HIGH);
    break;
  case 4:
    digitalWrite(DIAMOND_1_PIN, HIGH);
    digitalWrite(DIAMOND_2_PIN, HIGH);
    digitalWrite(DIAMOND_3_PIN, HIGH);
    digitalWrite(DIAMOND_4_PIN, HIGH);
    break;
  case 5:
    digitalWrite(DIAMOND_1_PIN, HIGH);
    digitalWrite(DIAMOND_2_PIN, HIGH);
    digitalWrite(DIAMOND_3_PIN, HIGH);
    digitalWrite(DIAMOND_4_PIN, HIGH);
    digitalWrite(DIAMOND_5_PIN, HIGH);
    break;
  default:
    digitalWrite(DIAMOND_1_PIN, HIGH);
    digitalWrite(DIAMOND_2_PIN, HIGH);
    digitalWrite(DIAMOND_3_PIN, HIGH);
    digitalWrite(DIAMOND_4_PIN, HIGH);
    digitalWrite(DIAMOND_5_PIN, HIGH);
    break;
  }
}

void like_led_indicate(int num)
{
  like_led_clear();
  switch (num)
  {
  case 0:
    break;
  case 1:
    digitalWrite(LIKE_1_PIN, HIGH);
    break;
  case 2:
    digitalWrite(LIKE_1_PIN, HIGH);
    digitalWrite(LIKE_2_PIN, HIGH);
    break;
  case 3:
    digitalWrite(LIKE_1_PIN, HIGH);
    digitalWrite(LIKE_2_PIN, HIGH);
    digitalWrite(LIKE_3_PIN, HIGH);
    break;
  case 4:
    digitalWrite(LIKE_1_PIN, HIGH);
    digitalWrite(LIKE_2_PIN, HIGH);
    digitalWrite(LIKE_3_PIN, HIGH);
    digitalWrite(LIKE_4_PIN, HIGH);
    break;
  case 5:
    digitalWrite(LIKE_1_PIN, HIGH);
    digitalWrite(LIKE_2_PIN, HIGH);
    digitalWrite(LIKE_3_PIN, HIGH);
    digitalWrite(LIKE_4_PIN, HIGH);
    digitalWrite(LIKE_5_PIN, HIGH);
    break;
  default:
    digitalWrite(LIKE_1_PIN, HIGH);
    digitalWrite(LIKE_2_PIN, HIGH);
    digitalWrite(LIKE_3_PIN, HIGH);
    digitalWrite(LIKE_4_PIN, HIGH);
    digitalWrite(LIKE_5_PIN, HIGH);
    break;
  }
}
void settings()
{
  for (uint8_t i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "username")
    {
      String username_str = server.arg(i);
      if (username_str.length() != 0)
      {
        Serial.println(username_str);
        strncpy(username, username_str.c_str(), sizeof(username));
        preferences.putString("username",username);
        username_updated=true;
      }
    }
    else if (server.argName(i) == "post_type")
    {
      String type = server.arg(i);
      Serial.println(type);
      if (type.equals("latest"))
      {
        post_type = 0;
      }
      else if (type.equals("fixed"))
      {
        post_type = 1;
      }
      preferences.putInt("post_type",post_type);
    }
    else if (server.argName(i) == "num")
    {
      String num = server.arg(i);
      if (num.length() != 0)
      {
        num_posts = num.toInt();
        Serial.println(num_posts);
        preferences.putInt("num_posts",num_posts);
      }

      
    }
    else if (server.argName(i) == "post_hash")
    {
      String hash = server.arg(i);
      if (hash.length() != 0)
      {
        strncpy(post_hash, hash.c_str(), sizeof(post_hash));
        Serial.println(post_hash);
        preferences.putString("post_hash",post_hash);
      }
      
    }
  }
  force_update=true;
  server.send(200, "text/html", main_page);
}

void info(){
  DynamicJsonDocument doc(300);
  char postData[300];
  doc["type"]=post_type;
  doc["hash"]=post_hash;
  doc["num"]=num_posts;
  doc["username"]=username;
  doc["likes"]=like_count;
  doc["diamonds"]=diamond_count;
  doc["node"]=deso.getSelectedNodeUrl();
  doc["balance"]=balance/1000000000.0;

  serializeJson(doc, postData);
  //Serial.println(postData);
  server.send(200, "application/json", postData);
}

void default_settings()
{
  //strncpy(username, username_default, sizeof(username));
  //num_posts = 1;
  //post_type = 0;
  String user = preferences.getString("username",username_default);
  strncpy(username,user.c_str(),sizeof(username));
  num_posts = preferences.getInt("num_posts",1);
  post_type = preferences.getInt("post_type",0);
  String post_hash_str = preferences.getString("post_hash",post_hash_default);
  strncpy(post_hash,post_hash_str.c_str(),sizeof(post_hash));
}
void setup()
{
  preferences.begin("settings", false); 
  pinMode(LIKE_1_PIN, OUTPUT);
  pinMode(LIKE_2_PIN, OUTPUT);
  pinMode(LIKE_3_PIN, OUTPUT);
  pinMode(LIKE_4_PIN, OUTPUT);
  pinMode(LIKE_5_PIN, OUTPUT);

  pinMode(DIAMOND_1_PIN, OUTPUT);
  pinMode(DIAMOND_2_PIN, OUTPUT);
  pinMode(DIAMOND_3_PIN, OUTPUT);
  pinMode(DIAMOND_4_PIN, OUTPUT);
  pinMode(DIAMOND_5_PIN, OUTPUT);

  pinMode(BALANCE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  like_led_clear();
  diamond_led_clear();
  like_led_test();
  diamond_led_test();
  delay(500);
  like_led_clear();
  diamond_led_clear();

  digitalWrite(BALANCE_PIN, LOW);
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);

  digitalWrite(BUZZER_PIN, LOW);
  Serial.begin(9600);
  WiFi.enableSTA(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, wifi_pass);

  while (!WiFi.isConnected())
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("setting up MDNS responder");
  if (!MDNS.begin(HOSTNAME))
  { // http://esp32.local
    Serial.println("Error setting up MDNS responder!");
  }
  server.on("/", []()
            { server.send(200, "text/html", main_page); });
  server.on("/settings/", settings);
  server.on("/info/", info);
  server.begin();

  default_settings();
  deso.addNodePath("https://desocialworld.com", ISRG_Root_caRootCert);
  deso.addNodePath("https://diamondapp.com",Baltimore_Root_caRootCert );
  deso.addNodePath("https://node.deso.org", GTS_Root_caRootCert);

  deso.selectDefaultNode(0);
  int status = false;
  while (!status)
  {
    status = deso.updateSingleProfile(username, "", &profile1);
    if (!status)
    {
      Serial.println("Single profile error!");
      nextServer();
    }
  }
  Serial.printf("\nUser:%s\n", profile1.Username);
}

void loop()
{
  static unsigned long timer1 = -50000UL;
  static unsigned long timer2 = 0;
  static bool balance_led_status = false;
  if (WiFi.isConnected())
  {
    if (millis() - timer1 > 30000UL || force_update)//update period is every 30 seconds
    {
      force_update=false;
      timer1 = millis();
      Serial.println("Updating");
      digitalWrite(LED_BUILTIN, HIGH);

      int status_update_num_posts;
      int status_update_single_post;
      int diamond_count_temp = -1;
      int like_count_temp = -1;

      if (post_type == 0)
      {
        status_update_num_posts = deso.updateLastNumPostsForPublicKey(profile1.PublicKeyBase58Check, num_posts, &profile1);
        if (status_update_num_posts)
        {
          diamond_count_temp = profile1.lastNPostDiamonds;
          like_count_temp = profile1.lastNPostLikes;
        }
      }
      else if (post_type == 1)
      {
        status_update_single_post = deso.updateSinglePost(post_hash, true, 0, 0, profile1.PublicKeyBase58Check, false, &post1);
        if (status_update_single_post)
        {
          diamond_count_temp = post1.DiamondCount;
          like_count_temp = post1.LikeCount;
        }
      }

      deso.updateUsersBalance(profile1.PublicKeyBase58Check, &profile1);
      digitalWrite(LED_BUILTIN, LOW);

      if (diamond_count_temp >= 0)
      {
        Serial.printf("DIAMOND:%d\n", diamond_count_temp);
        if (diamond_count != diamond_count_temp)
        {
          int change = diamond_count_temp - diamond_count;
          diamond_count = diamond_count_temp;
          diamond_led_indicate(diamond_count);

          if (change > 0)
          {
            for (int i = 0; i < change; i++)
            {
              digitalWrite(BUZZER_PIN, HIGH);
              delay(100);
              digitalWrite(BUZZER_PIN, LOW);
              delay(50);
            }
          }
        }
      }
      if (like_count_temp >= 0)
      {
        Serial.printf("LIKE:%d\n", like_count_temp);
        if (like_count != like_count_temp)
        {
          like_count = like_count_temp;
          like_led_indicate(like_count);
        }
      }

      double total = profile1.BalanceNanos + profile1.UnconfirmedBalanceNanos;
      Serial.printf("Balance:%f\n", total);
      if (total > 0)
      {
        if (total != balance)
        {
          balance = total;
          digitalWrite(BALANCE_PIN, HIGH);
          balance_led_status = true;
        }
      }
      timer2 = millis();
    }
    if (balance_led_status) // if balance led on, off after 2 seconds
    {
      if (millis() - timer2 > 2000)
      {
        digitalWrite(BALANCE_PIN, LOW);
        balance_led_status = false;
      }
    }
    server.handleClient();
    if(username_updated){
      username_updated=false;
      deso.updateSingleProfile(username, "", &profile1);
      Serial.printf("\nUser:%s\n", profile1.Username);
    }
    delay(1);
    
  }
}


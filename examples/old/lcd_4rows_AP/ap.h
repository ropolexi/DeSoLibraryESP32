#include <Arduino.h>
String meta_str="<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
const char postForms[] PROGMEM = R"(
<html>
<title>
DeSo Dashbaord
</title>
<head> 
 <meta name="viewport" content="width=device-width, initial-scale=1.0"> 
<style></style>
</head>
<body>
    <h1>ESP32 DESO Dashboard Settings</h1>
<h2>WiFi Settings</h2>
    <form method="POST" action="/wifi/">
        <table width="100%">
        <tr><td>SSID</td><td><input type="text" name="SSID" id="SSID" value=""></td></tr>
        <tr><td>PASS</td><td><input type="password" name="PASS" id="PASS" value=""></td></tr>
        <tr><td></td><td><input type="submit" value="Submit"></td></tr>
        </table>
    </form>  
<h2>Profile Settings</h2>
<p>Type the username without @ </p>
    <form method="POST" action="/profile/">
        <table width="100%">
        <tr><td>Username</td><td><input type="text" name="username" id="username" value="" maxlength="20"></td></tr>
        <tr><td>Update User Holdings</td><td>
        <select name="usersStateless" id="usersStateless">
            <option value="1">Enable</option>
            <option value="0">Disable</option>     
        </select>
        </td></tr>
        <tr><td></td><td><input type="submit" value="Submit"></td></tr>
        </table>
    </form>  
</body>
</html>
)";
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>


IPAddress apIP(192,168,4,1);
DNSServer dnsServer;//----------------------------------
ESP8266WebServer server(80);

const char MAIN_page[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>
<body>

<center><h1>RMS7811i</h1></center>

<center><table border="1" bordercolor="blue" bgcolor="skyblue" height="400" width="400">
<td>
<center>
<h3>UPDATE CREDENTIALS</h3>


<form action="/AfterSubmit ">
  New SSID:<br>
  <input type="text" name="NewSSID" placeholder="Enter your New SSID" required>
  <br>
  Password:<br>
  <input type="password" name="NewPassword" placeholder="Enter your SSID Password">
  <br>
  Authentication Token<br>
  <input type="text" name="AuthenticationToken" placeholder="Enter your BLYNK Auth" required>
  <br><br>
  <input type="submit" value="Submit">
</form><br><br>

</div>
</table>
</center> 
</td>
</center>
</body>
</html>

)=====";
//-------------------------------------------------------------------------------


void setup() {
 // WiFi.mode(WIFI_AP);//------------------------------------
  //WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("DNSServer CaptivePortal Test");

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
 dnsServer.start(53, "*", apIP);//----------------------------

  // replay to all requests with same HTML
  server.onNotFound([]() {
    server.send(200, "text/html",MAIN_page);
  });
  server.on("/AfterSubmit", []() {
    String content = "<!DOCTYPE HTML><html><h1>!!!UPDATED!!!<h1><br><br><h3>Restarting...<h3><br><br><h4>Please Restart Manually for better functionality<h4></html>";
     server.send(200, "text/html", content);
      });
  server.begin();
}

void loop() {
 dnsServer.processNextRequest();
  server.handleClient();
}
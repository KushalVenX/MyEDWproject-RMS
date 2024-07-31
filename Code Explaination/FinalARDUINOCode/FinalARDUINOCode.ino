#include <LiquidCrystal_I2C.h>         //Library for LCD Display 
LiquidCrystal_I2C lcd(0x3F,16,2); 
#include <ESP8266WiFi.h>               //Library for wifi and hotspot on and off
#include <ESP8266WebServer.h>          //Library for sending html
#include<EEPROM.h>                     //Library for EEPROM
#include <BlynkSimpleEsp8266.h>        //Library for Blynk.cloud server
//--------------------------------
#include <DNSServer.h>                 //For Captive Portal Auto Connect to the server in AP Mode
//--------------------------------
IPAddress ManuallysetIP(192,168,4,1);  //setting manual ip address
DNSServer SignInRequiredMsg;           // user define variable for DNSServer
//-----------------------------------------
int AP_decider=1;
int AP_presentstate;
int AP_laststate;
int APdisable;
int AP_enablebutton=D4;  //Pin D4 for input signal
//--------------------------------------
int Apin = A0;           //A0 input signal to microcontroller- door is closed or open
int buzzer= D8;
int buttonStateAnalogSignal=0;
int Counter=0;
int seconds=0;
String DoorStatus;
String PresentState;
String LastState;
String instrucn="Stable State";
//--------------------------------------
//--------------Variables for EEPROM-------------------
int lengthofprevssid;
int lengthofprevpass;
int lengthofprevAuth;
char ssidchar;
char passchar;
char Authchar;
String newssid="";
String newpass="";
String newAuth="";
//----------------------------------------------------
//------------------------Variables to store Credentials from Web Server--------
String inputssid;
String inputpass;
String inputAuth;
//-------------------------------------------------------------------------------
//------------HTML Code Store in Variable----------------------------------------
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<center><h1>RMS7811i</h1></center>
<center><table border="1" bordercolor="blue" bgcolor="skyblue" height="400" width="400">
<td>
<center>
<h3>UPDATE CREDENTIALS</h3>
<form action="/AfterSubmit">
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


//-------------------------------------------------------------------------------
ESP8266WebServer server(80); //Server on port 80
//-------------------------------------------------------------------------------


void setup() {
EEPROM.begin(512);
ReadingDataFromEEPROM();
server.begin();
Serial.begin(9600);
pinMode(AP_enablebutton,INPUT_PULLUP);
pinMode(Apin,INPUT); 
pinMode(buzzer,OUTPUT);

lcd.backlight();  
lcd.begin();  
lcd.clear();
lcd.print("--System Is ON--");  // initial display of lcd
lcd.setCursor(3,1);// LCD Cursor set at- 3rd coloumn 1st row
lcd.print("RMS7811i");
delay(2000);
lcd.clear();
lcd.setCursor(0,0);// LCD Cursor set at- 0th coloumn 0th row
lcd.print("Your System is Now Monitered with IoT   ");
delay(1000);
for (int pos=0; pos<24;pos++)
  {
    lcd.scrollDisplayLeft();
    delay(500);
    }
}

void loop() {
//-----------------------AP Decider------------------------------------------
char newAuthCharrArray[newAuth.length()+1];
newAuth.toCharArray(newAuthCharrArray,newAuth.length()+1); //string to charr array
if(digitalRead(AP_enablebutton)== HIGH){
  AP_presentstate=1;
}
if(digitalRead(AP_enablebutton)== LOW){
  AP_presentstate=0;
}
if(AP_decider %2==0){
   lcd.setCursor(14,1);
   lcd.print("AP"); 
   APdisable--; 
    if (APdisable==0){
    AP_presentstate=0;
     }
}
if(AP_presentstate!= AP_laststate){
  if(AP_presentstate==0){
  AP_decider++;}
   if(AP_decider %2==0){
     APdisable=120;
     WiFi.disconnect();
     WiFi.softAP("RMS7811i","12345678");
     SignInRequiredMsg.start(53, "*", ManuallysetIP);
     MyServer();
   }
   if(AP_decider %2 != 0){
    WiFi.softAPdisconnect();
    WiFi.begin(newssid,newpass);
    lcd.setCursor(14,1);
    lcd.print("  ");//Clearing AP mode display
    Blynk.config(newAuthCharrArray,"blynk.cloud", 8080); 
   }
}
AP_laststate=AP_presentstate;
//--------------------------------------------------------------------------
buttonStateAnalogSignal= analogRead(Apin); //Analog Signal from photoCoupler
if (buttonStateAnalogSignal>700){
   DoorStatus="Open";
   PresentState=DoorStatus;
   }
else{
   DoorStatus="Close";
   PresentState=DoorStatus;
   instrucn="Stable State";
  }

if(PresentState != LastState){
    if(PresentState=="Open"){
      Counter++;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.print("Door is open");// display that door is currently open

    }
    if(PresentState =="Close"){
      digitalWrite(buzzer,LOW);
      seconds=seconds*0;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.print("Door Closed!");
      lcd.setCursor(0,1);
      lcd.print("DoorOpened:"); //display door counter- part1
      lcd.print(Counter);  //display door counter- part2
      delay(1000);
    }
LastState=PresentState;
  }
if(PresentState=="Open"){
      seconds++;
      delay(1000);
      lcd.setCursor(0,1); // second row of display
      lcd.print("Since:");
      lcd.print(seconds);
      lcd.print("s");
      if(seconds>=60){
        instrucn="Buzzer is ON";
        digitalWrite(buzzer,HIGH);
        }
}
if(PresentState =="Close"){
  delay(1000);
}
 
//----------------------
if (WiFi.status()!=WL_CONNECTED){
  lcd.setCursor(13,1);
  lcd.print("   ");  
}
if (WiFi.status()==WL_CONNECTED){
  lcd.setCursor(13,1);
  lcd.print("IoT");
  int buzzerindicatortoiot=seconds;
  Blynk.virtualWrite(V1,DoorStatus);//String Variable open or closed
  Blynk.virtualWrite(V2,seconds);//timer
  Blynk.virtualWrite(V3,Counter);//doorcounter
  Blynk.virtualWrite(V4,instrucn);
  Blynk.virtualWrite(V5,buzzerindicatortoiot);
  Blynk.run();//Run the Blynk library
 }
//-----------------


//--------------------------
//---------------------------
//----------------------for continously handle server when it request--
server.handleClient();
//-----------------------------
//-------------DNSServer Always available to AP mode-------
SignInRequiredMsg.processNextRequest();
//----------------------
Serial.println(newssid);
Serial.println(newpass);
Serial.println(newAuth);
}

//--------------functions---------------------------------------
void ReadingDataFromEEPROM(){
  //---------Reading SSID,PASS and AUTH from EEPROM to begin wifi IOT Connection---------
EEPROM.get(150,lengthofprevssid);//Storing lenth value of ssid to given variable
for(int i=0 ; i<lengthofprevssid; ++i){
  EEPROM.get(i,ssidchar);
  newssid += ssidchar;}

EEPROM.get(155,lengthofprevpass);//Storing lenth value of pass to given variable
for(int i=0 ; i<lengthofprevpass; ++i){
  EEPROM.get(i+50, passchar);
  newpass += passchar;}

EEPROM.get(160,lengthofprevAuth);//Storing lenth value of Auth to given variable
for(int i=0 ; i<lengthofprevAuth; ++i){
  EEPROM.get(i+100,Authchar);
  newAuth += Authchar;}

//-------------------------------------------------------------------------------------
}


void MyServer(){

  server.onNotFound( []() {  
 
     String s= MAIN_page;
      server.send(200, "text/html", s);
    });

server.on("/AfterSubmit", []() {
      
      inputssid = server.arg("NewSSID");
      inputpass = server.arg("NewPassword");
      inputAuth = server.arg("AuthenticationToken");
          //-------------writing to EEPROM---------------------------------------------
     EEPROM.put(150,inputssid.length());
     for(int i=0 ; i<inputssid.length(); ++i){
     EEPROM.put(i,inputssid[i]);}

     EEPROM.put(155,inputpass.length());
     for(int i=0 ; i<inputpass.length(); ++i){
     EEPROM.put(i+50,inputpass[i]);} 
  
     EEPROM.put(160,inputAuth.length());
     for(int i=0 ; i<inputAuth.length(); ++i){
     EEPROM.put(i+100,inputAuth[i]);}
     //------------------- 
     EEPROM.commit(); // SAVING/FLASHING EEPROM
     //-------------------
     String content = "<!DOCTYPE HTML><html><h1>!!!UPDATED!!!<h1><br><br><h3>Restarting...<h3><br><br><h4>Please Restart Manually for better functionality<h4></html>";
     server.send(200, "text/html", content);
     delay(2000);
     ESP.reset();
      });
}
//-------------------------------------------------------------------------------


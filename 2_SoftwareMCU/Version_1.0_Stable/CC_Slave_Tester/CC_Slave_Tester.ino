//VERSION INFORMATION
//-------------------
//Classic Car Signal - Slave Unit
//Version 1.0

//!!CHANGE LOCATION
#define location 1 //(1=FL 2=FR 3=RR 4=RL)
#define testmode 0
#define debugmode
#define TL 0
#define SL 2

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//Global Variables 
//!!CHANGE IP
IPAddress ip(192, 168, 0, 1); //(1=FL 2=FR 3=RR 4=RL)
IPAddress gateway(192, 168, 0, 5);
IPAddress subnet(255, 255, 255, 0);

WiFiUDP Udp;

//!!!CHANGE SERIAL NUMBER
const char* ssid = "6880370";
const char* password = "6880370pwd";

uint8_t blink1;
uint8_t blink2;
uint8_t blink3;
uint8_t on1;
uint8_t on2;
uint8_t on3;
uint8_t st;
uint8_t blcnt=0;
uint8_t hrt=0;
unsigned int localUdpPort = 4210;
String replys = ip.toString();
char reply[16];


void setup() {
  uint8_t concnt=0;

  //Initialize vars as needed
  st=0;
  
  //Start Fresh Configuration
  ESP.eraseConfig();
  WiFi.persistent(false);
  
  //Setup Serial Port for Diagnostic purposes
  Serial.begin(115200);
  Serial.println();
  Serial.println("CC_Slave boot sequence:");
  Serial.println("Serial Communication... Done");
  
  //Setup pins
  Serial.print("Set pins to output mode...");
  pinMode(TL,OUTPUT);
  pinMode(SL,OUTPUT);
  digitalWrite(TL,HIGH);
  
  Serial.println("Done.");

  //Wifi Setup and Connection
  delay(500);
  Serial.print("Setting Station Mode...");WiFi.mode(WIFI_STA);Serial.println("Done.");
  Serial.print("Setting IP/GT/NM...");  WiFi.config(ip, gateway, subnet);Serial.println("Done.");
  Serial.print("Connecting_");WiFi.begin(ssid, password);
  concnt=0;
  while (WiFi.status() != WL_CONNECTED) {
    if(concnt > 20){
       Serial.print("- can't connect, restarting");    
       ESP.restart();
    }
    delay(500);
    Serial.print(".");
    concnt++;
    digitalWrite(TL,!digitalRead(TL));
  }
  digitalWrite(TL,LOW);
  Serial.print(WiFi.localIP());Serial.println("-Done.");

  //Setting up UDP
  Serial.print("Setting UDP communication...");
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  
  //Setup Location Specific
  Serial.print("Determining Location...");Serial.print(location);Serial.println(" ...Done");
  switch (location) {
  case 1: //Left Front
    on1=9;
    on2=9;
    on3=9;
    blink1=6;
    blink2=5;
    blink3=2;
    break;
  case 2: //Right Front
    on1=9;
    on2=9;
    on3=9;
    blink1=6;
    blink2=4;
    blink3=1;
    break;
  case 3: //Right Rear
    on1=5;
    on2=3;
    on3=9;
    blink1=6;
    blink2=4;
    blink3=1;
    break;
  case 4: //Left Rear
    on1=4;
    on2=3;
    on3=9;
    blink1=6;
    blink2=5;
    blink3=2;
    break;
  default:
    on1=9;
    on2=9;
    on3=9;
    blink1=9;
    blink2=9;
    blink3=9;
    break;
  }
  Serial.println(" ...Done");

  //Done Go to program
  Serial.println("Boot Complete. Running program.");
}

void loop() {
  uint8_t st_bl;
  char incomingPacket[255];
  String incomingString;
  String stateString;
  
  int packetSize = Udp.parsePacket();

  if (packetSize) {
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    incomingString=String(incomingPacket);
    Serial.printf("Received %d bytes from %s, port %d, ", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    Serial.println("UDP packet contents:"+incomingString);
    if (incomingString[0]=='!') {
      hrt=0;    
      stateString = incomingString.substring(1,incomingString.indexOf("@"));
      st=stateString.toInt();
      Serial.print("NewState:"+String(st)+" Ack:");

      replys.toCharArray(reply,16);
      Serial.println(reply);
      Udp.beginPacket(gateway,localUdpPort);
      Udp.write(reply);
      Udp.endPacket();
    }
  }

  if (st>9) { 
    digitalWrite(TL,HIGH);
    st_bl=st-10;
  } 
  else {
    digitalWrite(TL,LOW);
    st_bl=st;
  }
  
  if ((st_bl==blink1)||(st_bl==blink2)||(st_bl==blink3)){
    blcnt++;
    if (blcnt>10) {
      digitalWrite(SL,!digitalRead(SL));
      blcnt=0;
    }
  }
  else if ((st_bl==on1)||(st_bl==on2)||(st_bl==on3)){
    digitalWrite(SL,HIGH);
    blcnt=0;
  }
  else {
    blcnt=0;
    digitalWrite(SL,LOW);
  }

  hrt++;
  
  if (hrt>40) {
    Serial.println("STATE Request Timeout - Restarting!");    
       ESP.restart();
  }

  delay(50);  

}

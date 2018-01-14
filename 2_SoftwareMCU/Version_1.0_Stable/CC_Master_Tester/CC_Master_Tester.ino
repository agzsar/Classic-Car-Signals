//VERSION INFORMATION
//-------------------
//Classic Car Signal - Master Unit
//Calin Raszga
//Version 1.0

#define debugmode
#define ALARM 5
#define ACTIVITY 4
#define CANCEL 2
#define BRAKE 13
#define RIGHT 12
#define LEFT 14

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

IPAddress ip1(192, 168, 0, 1);
IPAddress ip2(192, 168, 0, 2);
IPAddress ip3(192, 168, 0, 3);
IPAddress ip4(192, 168, 0, 4);
IPAddress ip(192, 168, 0, 5);
IPAddress gateway(192, 168, 0, 5);
IPAddress subnet(255, 255, 255, 0);

WiFiUDP Udp;

//!! CHANGE SERIAL NUMBER
const char* ssid = "6880370";
const char* password = "6880370pwd";

//SLAVE ADDRESSES
const char* LF = "192.168.0.1";
const char* RF = "192.168.0.2";
const char* RR = "192.168.0.3";
const char* LR = "192.168.0.4";

uint8_t st = 0;
uint8_t bst = 0;
uint8_t servcounter = 0;
uint8_t leftcounter = 0;
uint8_t rightcounter = 0;
uint8_t lrcounter = 0;
uint8_t cancelcounter = 0;
uint8_t cancellongcounter = 0;
uint8_t brakecounter = 0;
bool left = false;
bool right = false;
bool lr = false;
bool cancellong = false;
bool cancel = false;
bool brake = false;
bool tailon = false;
uint8_t slavecnt = 1;
unsigned int localUdpPort = 4210;
uint16_t timer1=0;
uint16_t timer2=0;
uint16_t timer3=0;
uint16_t timer4=0;
bool noreset = false;

void setup() {
  
  ESP.eraseConfig();
  WiFi.persistent(false);

  //Init super-important variables
  noreset=false;
  
  //Setup Serial Port for Diagnostic purposes
  Serial.begin(115200);
  Serial.println();
  Serial.println("CC_Master boot sequence:");
  Serial.println("Serial Communication... Done");
  
  //Setup pins
  Serial.print("Set pins to proper mode...");
  pinMode(ACTIVITY,OUTPUT);
  pinMode(ALARM,OUTPUT);
  digitalWrite(ACTIVITY,HIGH);
  digitalWrite(ALARM,HIGH);
  pinMode(LEFT,INPUT_PULLUP);
  pinMode(RIGHT,INPUT_PULLUP);
  pinMode(BRAKE,INPUT_PULLUP);
  pinMode(CANCEL,INPUT_PULLUP);
  Serial.println("Done.");
  
  //Wifi Setup and Connection
  delay(500);
  Serial.print("Setting Access Point Mode...");WiFi.mode(WIFI_AP);Serial.println("Done.");
  Serial.print("Setting Access Point IP/GT/NT...");WiFi.softAPConfig(ip, gateway, subnet);Serial.println("Done.");
  Serial.print("Starting SSID Broadcast...");WiFi.softAP(ssid, password);Serial.println("Done.");

 //Check No-Reset Condition
  noreset = !(digitalRead(BRAKE));
  Serial.print("NoResetCondition:");Serial.println(noreset);
  
  //Setting up UDP
  Serial.print("Setting UDP communication...");
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  
  //Done Go to program
  Serial.println("Boot Complete. Running program.");
  digitalWrite(ACTIVITY,LOW);
  digitalWrite(ALARM,LOW);
}

void loop() {

 int packetSize = Udp.parsePacket();
 char incomingPacket[255];
 String remote;
 
 if (packetSize) {
  int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
  remote=String(incomingPacket);
  if (remote==LF) {
    Serial.println("Heartbeat from:"+remote);
    timer1=0;
  }
  else if (remote==RF) {
    Serial.println("Heartbeat from:"+remote);
    timer2=0;
  }
  else if (remote==RR) {
    Serial.println("Heartbeat from:"+remote);
    timer3=0;
  }
  else if (remote==LR) {
    Serial.println("Heartbeat from:"+remote);
    timer4=0;
  }
  else {
    Serial.println("Unknown Heartbeat from:"+remote);
    timer4=0;
  } 
 }
  
  //service every half second.
  servcounter++;
  if (servcounter>=10) {
    service();
    servcounter=0;
  }

  //service buttons
  left = service_btn(&leftcounter,!digitalRead(LEFT),4);
  right = service_btn(&rightcounter,!digitalRead(RIGHT),4);
  cancel = service_btn(&cancelcounter,!digitalRead(CANCEL),4);
  cancellong = service_btn(&cancellongcounter,!digitalRead(CANCEL),40);
  brake = service_btn(&brakecounter,!digitalRead(BRAKE),4);
  lr = service_btn(&lrcounter,!digitalRead(LEFT) && !digitalRead(RIGHT),2);

  //state machine
  switch (st) {
  case 0:   //ALLOFF - Signal Case Handling
            if (lr)          {st = 6; }
            else if (left)   {st = 2; }
            else if (right)  {st = 1; }
            else if (brake)  {st = 3; }
            else             {st = st;}
    
            //Tail Light Case Handling
            if (cancellong) {
              tailon = !tailon;
            }
            break;
  case 1:   //RT - Signal Case Handling
            if (brake)         {st = 4; }
            else if (cancel)   {st = 0; }
            else               {st = st;}
            break;

  case 2:   //LT - Signal Case Handling
            if (brake)         {st = 5; }
            else if (cancel)   {st = 0; }
            else               {st = st;}
            break;

  case 3:   //BRK - Signal Case Handling
            if (cancel)        {st = 0; }
            else if (left)     {st = 5; }
            else if (right)    {st = 4; }
            else               {st = st;}
            break;

  case 4:   //RT+BRK - Signal Case Handling
  case 5:   //LT+BRK - Signal Case Handling
            if (cancel)        {st = 0; }
            else               {st = st;}
            break;

  case 6:   //HAZ - Signal Case Handling
            if (cancel)        {st = 0; }
            else               {st = st;}
            break;
            
  default:  st=0;
            break;
  }

  if (tailon) {
    bst = st+10;
  }
  else {
    bst = st;
  }

  
//  #ifdef debugmode
//    Serial.println(String(left)+String(right)+String(brake)+String(cancel)+String(lr)+String(cancellong)+" "+String(st)+" "+String(bst));
//  #endif

  service_slv(&slavecnt,bst);

  
  if (timer1<1000) {
    timer1++;
  }
  if (timer2<1000) {
    timer2++;
  }
  if (timer3<1000) {
    timer3++;
  }
  if (timer4<1000) {
    timer4++;
  }
  

  if (((timer1>300)||(timer2>300)||(timer3>300)||(timer4>300))&&!noreset) {
    Serial.println("RESET! - Slave Timeout:"+String(timer1)+" "+String(timer2)+" "+String(timer3)+" "+String(timer4));
        ESP.restart();
  }

  delay(50);
}

bool service_slv(uint8_t *counter, uint8_t state){
  uint8_t localcounter = *counter;
  uint8_t i=0;
  char reply[10];
  String replys="";
  bool result = false;

  replys ="!"+ String(bst)+"@";
  replys.toCharArray(reply,10);


  switch (localcounter) {
  case 1:
         Udp.beginPacket(ip1,localUdpPort);
         Udp.write(reply);
         Udp.endPacket();
         localcounter++;
         break;
  case 2:
         Udp.beginPacket(ip2,localUdpPort);
         Udp.write(reply);
         Udp.endPacket();
         localcounter++;
         break;
  case 3:
         Udp.beginPacket(ip3,localUdpPort);
         Udp.write(reply);
         Udp.endPacket();
         localcounter++;
         break;
  case 4:
         Udp.beginPacket(ip4,localUdpPort);
         Udp.write(reply);
         Udp.endPacket();
         localcounter=1;
         break;
  default: 
         localcounter=0;
         break;
  }
  #ifdef debugmode
    Serial.println("UDP_Packet_Out:"+String(*counter)+" "+replys);
  #endif
  *counter = localcounter;
}

bool service_btn(uint8_t *counter, int input, uint8_t limit){
  uint8_t localcounter=*counter;
  bool btn = false;
  
  if ((input)&&(localcounter<limit)) {
    localcounter++;
    btn = false;
  }
  else if ((input)&&(localcounter>=limit))
  {
    localcounter = 0;
    btn = true;
  }
  else if ((!input)&&(localcounter>0)) {
    localcounter--;
    btn = false;
  }
  *counter = localcounter;
  return btn;
}

void service() {
  //Alarm if not enough clients connected
  Serial.println("Clients:"+String(WiFi.softAPgetStationNum())+" "+String(timer1)+" "+String(timer2)+" "+String(timer3)+" "+String(timer4));
  if ((timer1>50)||(timer2>50)||(timer3>50)||(timer4>50)) {
   digitalWrite(ALARM,HIGH);    
  }
  else{
    digitalWrite(ALARM,LOW);    
  }

  //Activity flash if state is not 0 or 10.
  if (bst==0){
    digitalWrite(ACTIVITY,LOW);
  }
  else if (bst==10) {
    digitalWrite(ACTIVITY,HIGH);
  }
  else {
    digitalWrite(ACTIVITY,!digitalRead(ACTIVITY));    
  }
}


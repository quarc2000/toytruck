#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <HTTPClient.h>
#include <ArduinoUniqueID.h>
#include <PubSubClient.h>

#include "motor.h"
#include "drive.h"
#include "usensor.h"
#include "light.h"
#include "dynamics.h"
#include "secrets.h"

// Define pin-name and GPIO#. Pin # are fixed but can ofcourse be named differently
// Motor
#define M1E_PIN 2
#define M1F_PIN 4
#define M1R_PIN 5

// Steering
#define SENABLE 33
#define SRIGHT 27
#define SLEFT 23

// US sensors
#define TFRONT 16 // ECHO1
#define EFRONT 34
#define TLEFT 26 // ECHO3
#define ELEFT 25
#define TREAR 19 // ECHO4
#define EREAR 18
#define TRIGHT 17 // ECHO2
#define ERIGHT 35

AsyncWebServer server(80);

String cpuid; // The unique hw id, actually arduino cpu-id

// Setup the needed components
Dynamics dynamics;
Usensor leftdistance(TLEFT, ELEFT);
Usensor rightdistance(TRIGHT, ERIGHT);
Usensor frontdistance(TFRONT, EFRONT);
Usensor reardistance(TREAR, EREAR);
Motor motor(M1E_PIN, M1F_PIN, M1R_PIN);
Drive drive(motor);
Motor steering(SENABLE, SLEFT, SRIGHT);
Light light;
float frontDist = 100;
float rearDist = 100;
int loopcount = 0;
// Function for reading uniuque chipid, to keep track of logs
String uids()
{
  String uidds;
  for (size_t i = 0; i < UniqueIDsize; i++)
  {
    if (UniqueID[i] < 0x10)
    {
      uidds = uidds + "0";
    }
    uidds = uidds + String(UniqueID[i], HEX);
  }
  return uidds;
}

void mqttlog(String msg){
  WiFiClient wificlient;
  PubSubClient mqttClient(wificlient);
  mqttClient.setServer("192.168.2.2", 1883);
  mqttClient.connect(cpuid.c_str());
  String topic = cpuid + "/logging";
  mqttClient.publish(topic.c_str(),msg.c_str());
}

String postlog(String msg)
{
  WiFiClient wificlient; // Used for sending http to url
  HTTPClient http;

  msg.replace(" ", "%20");

  String url = "http://192.168.2.2/post_log.php?chipid=" + cpuid + "&msg=" + msg;
  http.begin(wificlient, url);
  int httpResponseCode = http.GET();
  String payload = "{}";

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  return payload;
}

void setup()
{

  Serial.begin(9600);
  cpuid = uids(); // Read the unique boxid
  Serial.print("Hello from ");
  Serial.println(cpuid);

  IPAddress local_IP(192, 168, 2, 104);

  if (cpuid.startsWith("64b7084cff5c"))
  {
    IPAddress local_IP(192, 168, 2, 103);
  }

  IPAddress gateway(192, 168, 2, 1);
  IPAddress subnet(255, 255, 0, 0);
  IPAddress primaryDNS(8, 8, 8, 8);
  IPAddress secondaryDNS(8, 8, 4, 4); // optional
  WiFi.mode(WIFI_STA);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(ssid, password);

  Serial.println("");

  // Wait for connection
  // Connection-info in secrets.h
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "Hi! I am ESP32 in car running on " + cpuid); });

  AsyncElegantOTA.begin(&server); // Start ElegantOTA
  server.begin();
  postlog("OTA server started");
  postlog("ATMT started!");
  postlog("Mac: " + String(WiFi.macAddress()));
  frontdistance.SetUp();
  reardistance.SetUp();
  rightdistance.SetUp();
  leftdistance.SetUp();
  motor.SetUp();
  steering.SetUp();
  light.SetUp();
  dynamics.SetUp();

  Serial.println("Setup is done!");
}

void driveStrategy(){
  drive.Forward(1);
  delay(1000);
  drive.Stop();
  delay(1000);
  drive.Reverse(1);
  delay(1000);
  drive.Stop();
  light.Test();
  delay(5000);
}

bool somethingAhead() {
 return frontDist > 5 && frontDist < 20;
}

bool somethingBehind() {
 return rearDist > 5 && rearDist < 20;
}

void strategy() {
 if(somethingAhead()){
  drive.Reverse(1);
  light.Test();
 } else if (somethingBehind) { 
  drive.Forward(1);
  light.Test();
 } else {
  drive.Stop();
  light.Off();
 }
}

void loop()
{
  frontDist = frontdistance.GetDistance(); // update fron dist
  rearDist = reardistance.GetDistance(); // update fron dist
  strategy();
  delay(100);
}
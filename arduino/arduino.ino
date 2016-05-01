#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include <WebSockets.h>
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>

#include <Firebase.h>

#include <ArduinoJson.h>

#include <Ticker.h>

#include <stdio.h>
#include <string.h>

#define LED     D0        // Led in NodeMCU at pin GPIO16 (D0).
#define BUTTON  D3        // flash button at pin GPIO00 (D3)

#define JSON_BUFFER_SIZE (10*4)

// AP mode: local access
const char* ap_ssid     = "esp8266";
const char* ap_password = "123456789";

// STA mode: router access
char sta_ssid[25] = ""; // "D-Link DSL-3580L";
char sta_password[25] = ""; // "pippolo1234";
char firebase_url[50] = "";

void setup_ap_mode(void);
void setup_sta_mode(void);

// create firebase client
Firebase fbase = Firebase(""); // "ikka.firebaseIO.com"

// create sebsocket server
WebSocketsServer webSocket = WebSocketsServer(81);

Ticker flipper;

int mode = 1;
int flip_mode = 1;

int count = 0;

void setflip_mode(int mode) {
  flip_mode = mode;
}

void flip(void) {
  int trig;

  if (flip_mode != 2) {
    trig = (flip_mode == 0)?(1):(5);
    if (count >= trig) {
      int state = digitalRead(LED);
      digitalWrite(LED, !state); // set pin to the opposite state
      count = 0;
    }
    ++count;
  } else {
    digitalWrite(LED, 0);
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      setup_sta_mode();
      break;
    case WStype_CONNECTED: {
      setflip_mode(2);
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

      // send message to client
      webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
  
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject((char*)payload);
      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
      }
      const char* ssid = root["ssid"];
      strcpy(sta_ssid, ssid);
      Serial.printf("sta_ssid %s\n", sta_ssid);
      const char* password = root["password"];
      strcpy(sta_password, password);
      Serial.printf("sta_password %s\n", sta_password);
      const char* firebase = root["firebase"];
      strcpy(firebase_url, firebase);
      Serial.printf("firebase_url %s\n", firebase_url);
      const char* data = root["data"];
      Serial.printf("data %s\n", data);
      break;
    }
}

void setup_ap_mode() {

  // static ip for AP mode
  IPAddress ip(192,168,2,1);

  mode = 0;
  setflip_mode(0);
  Serial.printf("connecting mode %d\n", mode);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_AP_STA);

  WiFi.softAPConfig(ip, ip, IPAddress(255,255,255,0));
  WiFi.softAP(ap_ssid, ap_password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.println("AP mode enabled");
  Serial.print("IP address: ");
  Serial.println(myIP);
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void setup_sta_mode() {
  int cnt;
  mode = 1;
  setflip_mode(1);
  Serial.printf("connecting mode %d\n", mode);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_AP_STA);

  Serial.printf("sta_ssid: %s\n", sta_ssid);
  Serial.printf("sta_password: %s\n", sta_password);
  WiFi.begin(sta_ssid, sta_password);
  cnt = 0;
  while ((WiFi.status() != WL_CONNECTED) && (cnt++<10)){
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
    fbase = Firebase(firebase_url);
  } else {
    Serial.println();
    Serial.println("not connected to router");
    setup_ap_mode();
  }
}

void setup() {

  mode = 1;

  pinMode(LED, OUTPUT);   // LED pin as output.
  pinMode(BUTTON, INPUT);   // 
  Serial.begin(115200);

  flipper.attach(0.1, flip);

  Serial.println();
  Serial.println("Starting");
  if (mode == 0) {
    setup_ap_mode();
  } else if (mode == 1) {
    setup_sta_mode();
  }
}

void loop() {
  int in;
  String str;

  if (mode == 0) {
    webSocket.loop();
  } else if (mode == 1) {
    FirebaseGet get = fbase.get("");

    // get json from the root of the firebase
    if (get.error()) {
        Serial.println("Firebase get failed");
        Serial.println(get.error().message());
        return;
    }
    delay(500);

    // Serial.println(get.json()["settings"]);
    // get object data
    String data = get.json()["data"];
    String settings = ""; //get.json()["settings"];
    delay(500);

    digitalWrite(LED, (data[0]=='0'));
    Serial.printf("%x %x\n", data[0], settings[0]);
    // Serial.print(setting[0]);
    in = digitalRead(BUTTON);

#if 0
    str = "{\"button\": \"" + String(in) + "\"}";
    FirebaseSet set = fbase.set("ctrl/", str);
    if (set.error()) {
      Serial.println("Firebase set failed");
      Serial.println(set.error().message());  
      return;
    }
    delay(500);

  FirebasePush push = fbase.push("/logs", "{\".sv\": \"timestamp\"}");
  if (push.error()) {
      Serial.println("Firebase push failed");
      Serial.println(push.error().message());  
      return;
  }
#endif
  }
}


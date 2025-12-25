#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "Xuan Vu";
const char* password = "Xuanvu1502";

const char* mqtt_broker = "beb3a3a883cd4a39a0ba739381db1a6b.s1.eu.hivemq.cloud";
const int   mqtt_port   = 8883;
const char* mqtt_user   = "minhthai";
const char* mqtt_pass   = "Conan1306@@@"; // đúng mật khẩu bạn vừa reset

const char* SUB_LIGHT = "MQTT_ESP32/LIGHT";

#define RELAY_PIN 26
#define RELAY_ACTIVE_LOW 0

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setRelay(bool on){
  if(RELAY_ACTIVE_LOW) digitalWrite(RELAY_PIN, on ? LOW : HIGH);
  else                digitalWrite(RELAY_PIN, on ? HIGH : LOW);
}

void callback(char* topic, byte* payload, unsigned int length){
  Serial.print("TOPIC: "); Serial.println(topic);
  Serial.print("MSG: ");
  String msg;
  for(unsigned int i=0;i<length;i++){
    char c = (char)payload[i];
    Serial.print(c);
    msg += c;
  }
  Serial.println();

  if(String(topic) != SUB_LIGHT) return;

  DynamicJsonDocument doc(128);
  if(deserializeJson(doc, msg) != DeserializationError::Ok){
    Serial.println("JSON parse fail");
    return;
  }

  String v = String(doc["light"] | "");
  v.toLowerCase();

  if(v == "on"){
    setRelay(true);
    Serial.println("RELAY ON");
  }else if(v == "off"){
    setRelay(false);
    Serial.println("RELAY OFF");
  }else{
    Serial.println("Invalid light value");
  }
}

void setup_wifi(){
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK: " + WiFi.localIP().toString());
}

void reconnect_mqtt(){
  while(!client.connected()){
    String cid = "esp32-" + WiFi.macAddress();
    Serial.print("Connecting MQTT...");
    if(client.connect(cid.c_str(), mqtt_user, mqtt_pass)){
      Serial.println("OK");
      client.subscribe(SUB_LIGHT);
      Serial.println("Subscribed: MQTT_ESP32/LIGHT");
    }else{
      Serial.print("Fail rc="); Serial.println(client.state());
      delay(1500);
    }
  }
}

void setup(){
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false);

  // test relay 1 lần khi khởi động
  setRelay(true);  delay(300);
  setRelay(false); delay(300);

  setup_wifi();

  espClient.setInsecure();
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  reconnect_mqtt();
}

void loop(){
  if(!client.connected()) reconnect_mqtt();
  client.loop();
}

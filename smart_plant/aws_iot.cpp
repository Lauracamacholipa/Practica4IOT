#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "config.h"
#include "secrets.h"
#include "pump.h"

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

String shadowUpdateTopic;
String shadowDeltaTopic;
String telemetryTopic;

void buildTopics() {
    String base = "$aws/things/" + String(THING_NAME) + "/shadow";

    shadowUpdateTopic = base + "/update";
    shadowDeltaTopic  = base + "/update/delta";

    telemetryTopic =
      "macetas/" + String(THING_NAME) + "/telemetry";
}

void callback(char* topic,
              byte* payload,
              unsigned int length)
{
    String msg;

    for(int i=0;i<length;i++)
        msg += (char)payload[i];

    Serial.println("MQTT recibido:");
    Serial.println(msg);

    StaticJsonDocument<512> doc;

    if(deserializeJson(doc,msg))
        return;

    if(String(topic)==shadowDeltaTopic){

        JsonObject state =
        doc["state"].as<JsonObject>();

        if(state.containsKey("irrigation")){

            bool cmd =
            state["irrigation"].as<bool>();

            if(cmd)
                turnPumpOn();
            else
                turnPumpOff();
        }
    }
}

void connectWiFi() {
    Serial.print("Conectando WiFi");

    WiFi.begin(WIFI_SSID,WIFI_PASS);

    while(WiFi.status()!=WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi OK");
}

void reconnectMQTT() {
    while(!client.connected()){

        Serial.println("Conectando AWS MQTT...");

        if(client.connect(CLIENT_ID)){
            Serial.println("MQTT conectado");

            client.subscribe(
                shadowDeltaTopic.c_str()
            );
        }
        else{
            Serial.println(client.state());
            delay(5000);
        }
    }
}

void initAWS() {
    buildTopics();

    connectWiFi();

    wifiClient.setCACert(
        AMAZON_ROOT_CA1
    );

    wifiClient.setCertificate(
        CERTIFICATE
    );

    wifiClient.setPrivateKey(
        PRIVATE_KEY
    );

    client.setServer(
        MQTT_BROKER,
        MQTT_PORT
    );

    client.setCallback(callback);
}

void loopAWS() {
    if(!client.connected())
        reconnectMQTT();

    client.loop();
}

void publishTelemetry(int humidity,
                      bool irrigation)
{
    if(!client.connected())
        return;

    StaticJsonDocument<256> doc;

    doc["thing_name"] = THING_NAME;
    doc["humidity"] = humidity;
    doc["irrigation"] = irrigation;

    char buffer[256];

    serializeJson(doc,buffer);

    client.publish(
        telemetryTopic.c_str(),
        buffer
    );

    Serial.println(buffer);
}
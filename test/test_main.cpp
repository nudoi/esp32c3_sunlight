/**
 * @file main.cpp
 * 
 * @brief Send sunlight sensor data to MQTT broker via WiFi network.
 * 
 * @note This project is licensed under the MIT License.
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Si115X.h>
#include "config.h"

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

Si115X si1151;

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()/*,mqtt_user,mqtt_pass*/)) {
      Serial.println("connected");
      //client.publish(topic, "START");
      //client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      char buf[256];
      wifiClient.lastError(buf,256);
      Serial.print("SSL error: ");
      Serial.println(buf);
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

const long interval = 1000;
unsigned long previousMillis = 0;

int count = 0;


void setup() {
    Serial.begin(SERIAL_BAUDRATE);

    Serial.println("====================");
    Serial.print("WiFi Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("====================");

    Serial.println("Si1151 Test");

    uint8_t conf[4];

    Wire.begin();

    if (!si1151.Begin())
        Serial.println("Si1151 is not ready!");
    else
        Serial.println("Si1151 is ready!");

    Serial.println("====================");

    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(mqtt_server);

    wifiClient.setCACert(root_ca);
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    Serial.println("You're connected to the MQTT broker!");

    Serial.println("====================");
}

void loop() {

    // WiFi connection check
    if (WiFi.status () != WL_CONNECTED) {
        Serial.println("WiFi connection lost!");
        Serial.println("Reconnecting...");

        WiFi.disconnect();
        WiFi.reconnect();

        Serial.println("WiFi connected.");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.println("====================");
    }

    // MQTT connection check
    if (!client.connected()) {
        reconnect();
    }

    // sensing data
    uint16_t ir = si1151.ReadHalfWord();
    uint16_t vi = si1151.ReadHalfWord_VISIBLE();
    uint16_t uv = si1151.ReadHalfWord_UV();

    String message = "IR: " + String(ir) + " VISIBLE: " + String(vi) + " UV: " + String(uv);

    
    // publish data to MQTT broker
    if (client.publish(topic, message.c_str())) {
        Serial.println("Publish success!");
    } else {
        Serial.println("Publish failed!");
    }

    Serial.println("====================");

    Serial.print("IR: ");
    Serial.println(ir);
    Serial.print("VISIBLE: ");
    Serial.println(vi);
    Serial.print("UV: ");
    Serial.println(uv);

    Serial.println("====================");

    delay(SENSING_RATE);
}

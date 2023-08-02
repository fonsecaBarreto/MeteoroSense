// Titulo: Integração HTTP
// Data: 30/07/2023
//.........................................................................................................................

#pragma once
#include <WiFi.h>
// #include <WiFiClientSecure.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include "conf.h"

/**** WIFI Client Initialisation *****/
WiFiClient wifiClient;

/**** Secure WiFi Connectivity Initialisation *****/
// WiFiClientSecure secureWifiClient;

/**** NTP Client Initialisation  *****/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

/**** MQTT Client Initialisation Using WiFi Connection *****/
PubSubClient mqttClient(wifiClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

int connectWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  // secureWifiClient.setCACert(root_ca);      // enable this line and the the "certificate" code for secure connection
  return 1;
}

/***** MQTT ****/

void callback(char *topic, byte *payload, unsigned int length)
{
  String incommingMessage = "";
  for (int i = 0; i < length; i++)
    incommingMessage += (char)payload[i];
  Serial.println("Recebimento confirmado [" + String(topic) + "]" + incommingMessage);
}

int sendMeasurementToMqtt(const char *payload)
{
  if (mqttClient.publish(mqtt_topic, payload, true))
  {
    Serial.println("Message publised [" + String(mqtt_topic) + "]: " + payload);
  }
  return 1;
}

int setupMqtt()
{
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);
  return 1;
}

int connectMqtt()
{
  Serial.print("Attempting to connect to the MQTT broker: ");
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-"; // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("connected");
      mqttClient.subscribe(mqtt_topic); // subscribe the topics here
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds"); // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  return 1;
}

/* Ntp Client */
int connectNtp()
{
  Serial.println("\nStarting ntp connection");
  timeClient.begin();
  return 1;
}
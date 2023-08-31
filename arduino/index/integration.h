// Titulo: Integração HTTP
// Data: 30/07/2023
//.........................................................................................................................

#pragma once
#include <WiFi.h>
// #include <WiFiClientSecure.h>
#include <NTPClient.h>
#include <PubSubClient.h>

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

int setupWifi(char* ssid, char*password)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Wifi: Configuração inicial de wifi");
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  // secureWifiClient.setCACert(root_ca);      // enable this line and the the "certificate" code for secure connection
  return 1;
}

/***** MQTT ****/

void callback(char *topic, byte *payload, unsigned int length)
{
  String incommingMessage = "";
  for (int i = 0; i < length; i++)
    incommingMessage += (char)payload[i];
  Serial.println("MQTT broker: Recebimento confirmado [" + String(topic) + "]" + incommingMessage);
}

bool sendMeasurementToMqtt(char *topic, const char *payload)
{
  bool sent = (mqttClient.publish(topic, payload, true));
  if (sent){
    Serial.println("MQTT broker: Message publised [" + String(topic) + "]: " + payload);
  } else {
    Serial.println("MQTT: Não foi possivel enviar");
  }
  return sent;
}

int setupMqtt(char* mqtt_server, int mqtt_port)
{
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);
  return 1;
}

void connectMqtt(char* mqtt_username, char* mqtt_password, char* mqtt_topic) {
  if (!mqttClient.connected()) {
    Serial.println("Desconectado");
    Serial.print("MQTT: Tentando conexão...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)){
      Serial.println("MQTT: Reconected");
      mqttClient.subscribe(mqtt_topic);
      return;
    }
    Serial.print("failed, rc=");
    Serial.print(mqttClient.state());
    return;
  }
  Serial.println("Conectado [" + String(mqtt_topic) + "]");
}

/* Ntp Client */
int connectNtp()
{
  Serial.println("NTP connection : Tentando conectar....");
  timeClient.begin();
  Serial.println("NTP connection : Conectado com sucesso.");
  return 1;
}
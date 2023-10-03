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

/***** WIFI ****/

int setupWifi(const char *contextName, char* ssid, char*password)
{
  Serial.printf("%s: Estabelecendo conexão inicial", contextName);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  // secureWifiClient.setCACert(root_ca);    // enable this line and the the "certificate" code for secure connection

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  
  Serial.printf("\n%s: Contectado com sucesso \n", contextName);
  Serial.printf("%s: IP address = %s \n", contextName, WiFi.localIP());
  return 1;
}

/***** MQTT ****/

/* void callback(char *topic, byte *payload, unsigned int length)
{
  String incommingMessage = "";
  for (int i = 0; i < length; i++)
    incommingMessage += (char)payload[i];
  Serial.println("MQTT broker: Recebimento confirmado [" + String(topic) + "]" + incommingMessage);
} */

bool sendMeasurementToMqtt(char *topic, const char *payload)
{
  bool sent = (mqttClient.publish(topic, payload, true));
  if (sent){
    Serial.println("  - MQTT broker: Message publised [" + String(topic) + "]: ");
  } else {
    Serial.println("  - MQTT: Não foi possivel enviar");
  }
  return sent;
}

bool connectMqtt(const char *contextName, char* mqtt_username, char* mqtt_password, char* mqtt_topic) {
  if (!mqttClient.connected()) {
    Serial.printf("%s: Tentando nova conexão...", contextName);
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)){
      Serial.printf("%s: Reconectado", contextName);
      mqttClient.subscribe(mqtt_topic);
      return true;
    }
    Serial.print("failed, rc=");
    Serial.print(mqttClient.state());
    return false;
  }
  Serial.printf("%s: Conectado [ %s ]", contextName, mqtt_topic);
  return true;
}

bool setupMqtt(const char *contextName, char* mqtt_server, int mqtt_port, char* mqtt_username, char* mqtt_password, char* mqtt_topic)
{
  Serial.printf("%s: Estabelecendo conexão inicial\n", contextName);
  mqttClient.setServer(mqtt_server, mqtt_port);
  // mqttClient.setCallback(callback);
  return connectMqtt(contextName, mqtt_username, mqtt_password, mqtt_topic);
}

/* Ntp Client */
int connectNtp(const char *contextName)
{
  Serial.printf("%s: Estabelecendo conexão inicial\n", contextName);

  timeClient.begin();

  while(!timeClient.update()) {
    Serial.print(".");
    delay(1000);
  }

  Serial.printf("%s: Conectado com sucesso. \n", contextName);
  return 1;
}
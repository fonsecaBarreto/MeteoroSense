// Autor: Lucas Fonseca
// Titulo: Integração HTTP
// Data: 01/06/2023
//.........................................................................................................................
#pragma once
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoMqttClient.h>

// wifi
const char* ssid = "Gabriel";
const char* password = "2014072276";
// rest api
const String API_URL = "http://192.168.0.173:3000/csv";
// mqtt api
const char broker[] = "192.168.0.173";
int port = 38298;
const char topic[] = "measurements";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

int connectWifi() {
  WiFi.mode(WIFI_STA);  //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  return 1;
}

int connectMqtt()
{
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port))
  {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  return 1;
}

int sendMeasurementToMqtt(const char *csv)
{
  mqttClient.poll();
  mqttClient.beginMessage(topic);
  mqttClient.print(csv);
  mqttClient.endMessage();
  return 1;
}


int sendMeasurementToHttp(const char* csv) {

  if (WiFi.status() != WL_CONNECTED) { return 0; }

  WiFiClient client;
  HTTPClient http;

  http.begin(client, API_URL);
  http.addHeader("Content-Type", "text/plain");

  Serial.println("\n[HTTP] POST : " + API_URL);

  int httpCode = http.POST(csv);
  Serial.printf("--> code: %d\n\n", httpCode);

  if (httpCode == HTTP_CODE_OK) {
    const String& payload = http.getString();
    Serial.println("received payload:\n<<");
    Serial.println(payload);
    Serial.println(">>");
  }

  http.end();

  return 1;
}
import mqtt from "mqtt";
import { storeMeasurement } from "../../controllers/storeMeasurement.js";

const clientId = `mqtt_${Math.random().toString(16).slice(3)}`
const connectUrl = process.env.MQTT_URL;
const topic = process.env.MQTT_TOPIC;

export function connectToMqtt() {

  const client = mqtt.connect(connectUrl, {
    clientId,
    clean: true,
    connectTimeout: 4000,
    reconnectPeriod: 1000,
    username: process.env.MQTT_USERNAME,
    password: process.env.MQTT_PASSWORD
  });

  client.on("connect", () => {
    console.log("Mqtt Connected");

    client.subscribe([topic], () => {
      console.log(`Subscribe to topic '${topic}'`);
    });

    client.on("message", (topic, payload) => {
      try {
        console.log(topic, new Date().toLocaleDateString()," : " , payload.toString());
        const dto = JSON.parse(payload.toString());
        dto.timestamp = new Date();
        storeMeasurement(dto);
      } catch (err) {
        console.log("Falha ao receber medição");
      }
    });
  });
}

import mqtt from "mqtt";
import { csvStringToJson } from "../../helpers/parsers.js";
import { storeMeasurement } from "../../controllers/storeMeasurement.js";

const clientId = `mqtt_${Math.random().toString(16).slice(3)}`;
const topic = "measurements";
export function connectToMqtt(connectUrl) {
  const client = mqtt.connect(connectUrl, {
    clientId,
    clean: true,
    connectTimeout: 4000,
    reconnectPeriod: 1000,
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

import mqtt from "mqtt";
import { storeMeasurement } from "../../controllers/storeMeasurement.js";

const protocol = 'mqtt'
const host = 'telemetria.macae.ufrj.br:1883'
const port = '1883'
const clientId = `mqtt_${Math.random().toString(16).slice(3)}`
const connectUrl = `${protocol}://${host}:${port}`
const topic = "/prefeituras/macae/estacoes/est001";

export function connectToMqtt() {

  const client = mqtt.connect(connectUrl, {
    clientId,
    clean: true,
    connectTimeout: 4000,
    reconnectPeriod: 1000,
    username: 'telemetria',
    password: 'kancvx8thz9FCN5jyq'
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

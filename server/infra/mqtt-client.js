const mqtt = require("mqtt");
const { mqttUrl: connectUrl } = require("../config/constants");
const { csvStringToJson } = require("../helpers/parsers");

const clientId = `mqtt_${Math.random().toString(16).slice(3)}`;
const topic = "measurements";

function connectToMqtt() {
  console.log("Connecting to mqtt", connectUrl);
  const client = mqtt.connect(connectUrl, {
    clientId,
    clean: true,
    connectTimeout: 4000,
    reconnectPeriod: 1000,
  });

  client.on("connect", () => {
    console.log("Connected");

    client.subscribe([topic], () => {
      console.log(`Subscribe to topic '${topic}'`);
    });

    client.on("message", (topic, payload) => {
      console.log("Received Message:", topic);
      console.log(new Date().toLocaleString(), " - data received:\n");
      const json = csvStringToJson(payload.toString());
      console.log(json);
    });
  });
}

module.exports = { connectToMqtt };

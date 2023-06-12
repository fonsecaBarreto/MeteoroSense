const mqtt = require("mqtt");
const protocol = "mqtt";
const host = "192.168.0.173";
const port = "38298";
const clientId = `mqtt_${Math.random().toString(16).slice(3)}`;
const connectUrl = `${protocol}://${host}:${port}`;

const topic = "measurements";
function connectToMqtt() {}

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
    console.log("Received Message:", topic, payload.toString());
  });
});

module.exports = { connectToMqtt };

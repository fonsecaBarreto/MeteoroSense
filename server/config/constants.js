const { NODE_ENV, MQTT_URL, DATABASE_URL } = process.env;

module.exports = {
  nodeEnv: NODE_ENV,
  mqttUrl: MQTT_URL,
  databaseUrl: DATABASE_URL,
};

const express = require('express');
const { connectToMqtt } = require('../infra/mqtt-client');
const { csvStringToJson } = require("../helpers/parsers");
const app = express()
const port = 3000

connectToMqtt();

app.use(express.text())

app.post('/csv', (req, res) => {
  console.log(new Date().toLocaleString(), " - data received:\n");
  const json = csvStringToJson(req.body);
  console.log(json);
  return res.send("ok");
})

app.listen(port, () => {
  console.log(`Listening on port ${port}`)
})
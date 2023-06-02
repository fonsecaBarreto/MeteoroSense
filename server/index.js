const express = require('express')
const { csvStringToJson } = require("./parsers");
const app = express()
const port = 3000

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
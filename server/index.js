const express = require('express')
const app = express()
const port = 3000

app.use(express.text())

app.post('/csv', (req, res) => {
  console.log("data received", req.body);
  return res.send("ok");
})

app.listen(port, () => {
  console.log(`Listening on port ${port}`)
})
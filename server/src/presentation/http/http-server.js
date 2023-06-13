import express from "express";
import { csvStringToJson } from "../../helpers/parsers.js";

export function startHttpServer(port) {
  const app = express();

  app.use(express.text());

  app.post("/csv", (req, res) => {
    console.log(new Date().toLocaleString(), " - data received:\n");
    const json = csvStringToJson(req.body);
    console.log(json);
    return res.send("ok");
  });

  app.listen(port, () => {
    console.log(`Listening on port ${port}`);
  });

}

import { getGlobalVariables } from "./constants.js";
import { connectDatabase } from "./infra/pg-adapter.js";
import { connectToMqtt } from "./presentation/mqtt/mqtt-client.js";
import { startHttpServer } from "./presentation/http/http-server.js";


async function main(){
  console.log("SIT")
  console.log(" - Loading global variables...")
  const constants = getGlobalVariables();
  console.log(" - Connecting to Database...");
  await connectDatabase();
  console.log(" - Connecting to mqtt...");
  connectToMqtt(constants.mqttUrl);
  console.log(" - Stargin http server...");
  const port = 3000;
  startHttpServer(port);
}

main();
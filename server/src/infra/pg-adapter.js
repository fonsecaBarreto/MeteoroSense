import pkg from "pg";
const { Client } = pkg;
import { runMigrations } from "./migrations/initial.js";

const client = new Client({
  host: "db",
  port: 5432,
  database: "sit-database",
  user: "postgres",
  password: "1234567",
});

export async function connectDatabase() {
  try {
    await client.connect();
    console.log('Connected to the database.');
    await runMigrations(client);
    console.log('Migrations done.');
    return client;
  } catch (err) {
    console.error("Error creating measurements table:", err);
  }
}

export default client;
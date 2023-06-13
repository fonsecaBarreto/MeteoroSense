import client from "../infra/pg-adapter.js"

export async function storeMeasurement( measurement ) {
    try {
      await client.query('SELECT * FROM measurements');
      await client.query(
        "INSERT INTO measurements (timestamp, wind_speed, rain_cc, humidity, temperature) VALUES ($1, $2, $3, $4, $5)",
        [
          measurement.timestamp,
          measurement.wind_speed,
          measurement.rain_cc,
          measurement.humidity,
          measurement.temperature,
        ]
      );
      console.log('Operations completed successfully.');
    } catch (err) {
      console.error('Error performing database operations:', err);
    }
  }
  
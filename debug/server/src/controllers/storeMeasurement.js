import client from "../infra/pg-adapter.js"

export async function storeMeasurement( measurement ) {
    try {
      await client.query('SELECT * FROM measurements');
      await client.query(
        "INSERT INTO measurements (timestamp, wind_speed, rain_cc, humidity, temperature, wind_dir) VALUES ($1, $2, $3, $4, $5, $6)",
        [
          measurement.timestamp ?? new Date(),
          measurement.wind_speed ?? 0,
          measurement.rain_cc ?? 0,
          measurement.humidity ?? 0,
          measurement.temperature ?? 0,
          measurement.wind_dir ?? 0
        ]
      );
      console.log('Operations completed successfully.');
    } catch (err) {
      console.error('Error performing database operations:', err);
    }
  }
  
import client from "../infra/pg-adapter.js"

export async function storeMeasurement( measurement ) {
    try {
      // await client.query('SELECT * FROM measurements');

      await client.query(
        "INSERT INTO measurements (timestamp, temperatura, umidade_ar,  velocidade_vento, rajada_vento, dir_vento, volume_chuva, pressao, uid) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)",
        [
          measurement.timestamp ?? new Date(),
          measurement.temperatura ?? 0,
          measurement.umidade_ar ?? 0,
          measurement.velocidade_vento ?? 0,
          measurement.rajada_vento ?? 0,
          measurement.dir_vento ?? 0,
          measurement.volume_chuva ?? 0,
          measurement.pressao ?? 0,
          measurement.uid,
        ]
      );
      console.log('Operations completed successfully.');
    } catch (err) {
      console.error('Error performing database operations:', err);
    }
  }
  
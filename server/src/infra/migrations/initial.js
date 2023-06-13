export const createTableQuery = `
  CREATE TABLE IF NOT EXISTS measurements (
    id SERIAL PRIMARY KEY,
    timestamp TIMESTAMP NOT NULL,
    temperature FLOAT,
    humidity FLOAT,
    rain_cc FLOAT,
    wind_speed FLOAT
  );
`;

export async function runMigrations(client) {
    try {
      await client.query(createTableQuery);
      console.log('Measurements table created successfully.');
    } catch (err) {
      console.error('Error creating measurements table:', err);
      throw err;
    }
  }
  

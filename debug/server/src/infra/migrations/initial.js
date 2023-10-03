export const createTableQuery = `
  CREATE TABLE IF NOT EXISTS measurements (
    id SERIAL PRIMARY KEY,
    timestamp TIMESTAMP NOT NULL,
    temperatura FLOAT,
    umidade_ar FLOAT,
    velocidade_vento FLOAT,
    rajada_vento FLOAT,
    dir_vento INTEGER,
    volume_chuva FLOAT,
    pressao FLOAT,
    uid VARCHAR(20)
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
  

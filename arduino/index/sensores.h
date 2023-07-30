#pragma once

int getWindDir();
void anemometerChange();
void pluviometerChange();
void DHTRead(float& hum, float& temp);
void BMPRead(float& press);
void beginBMP();
void beginDHT();
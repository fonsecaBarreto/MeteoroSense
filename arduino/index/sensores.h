#pragma once

int getWindDir();
void anemometerChange();
void pluviometerChange();
void DHTRead(float& hum, float& temp);
void BMPRead(float& press);
void beginBMP();
void beginDHT();


union  Sensors{
    char ch;
    struct {
        bool div  : 1;
        bool vvt  : 1;
        bool dht  : 1;
        bool bmp  : 1;
        bool aux1 : 1;
        bool aux2 : 1;
        bool aux3 : 1;
        bool aux4 : 1;
    } bits;
};
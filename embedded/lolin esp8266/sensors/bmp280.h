#ifndef BMP280_DRIVER_H
#define BMP280_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

class BMP280_Driver {
public:
    BMP280_Driver();
    
    bool begin(uint8_t address = 0x76);
    
    float readPressure();
    

    float readTemperature();
    
    float readAltitude(float seaLevelPressure = 1013.25);

    bool isConnected();

private:
    Adafruit_BMP280 bmp;
    bool initialized;
};

#endif
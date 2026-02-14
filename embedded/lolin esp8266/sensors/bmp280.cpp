#include "bmp280_driver.h"

BMP280_Driver::BMP280_Driver() : initialized(false) {
}

bool BMP280_Driver::begin(uint8_t address) {
    Serial.print("[BMP280] Initializing at address 0x");
    Serial.print(address, HEX);
    Serial.print("... ");
    
    if (!bmp.begin(address)) {
        Serial.println("FAILED!");
        initialized = false;
        return false;
    }
    
    //Configure sensor for optimal settings
    bmp.setSampling(
        Adafruit_BMP280::MODE_NORMAL,  
        Adafruit_BMP280::SAMPLING_X2,    
        Adafruit_BMP280::SAMPLING_X16,    
        Adafruit_BMP280::FILTER_X16,      
        Adafruit_BMP280::STANDBY_MS_500   
    );
    
    initialized = true;
    Serial.println("OK");
    
    //Serial print sensor info
    Serial.print("[BMP280] Chip ID: 0x");
    Serial.println(bmp.sensorID(), HEX);
    
    return true;
}

float BMP280_Driver::readPressure() {
    if (!initialized) {
        return 0.0;
    }
    return bmp.readPressure();
}

float BMP280_Driver::readTemperature() {
    if (!initialized) {
        return 0.0;
    }
    return bmp.readTemperature();
}

float BMP280_Driver::readAltitude(float seaLevelPressure) {
    if (!initialized) {
        return 0.0;
    }
    return bmp.readAltitude(seaLevelPressure); //We need this to calibrate Above Ground Level in .ino
}

bool BMP280_Driver::isConnected() {
    return initialized;
}
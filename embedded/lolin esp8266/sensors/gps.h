#ifndef GPS_DRIVER_H
#define GPS_DRIVER_H

#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h> //ESP8266

class GPS_Driver {
public:
    //TX to D2 and RX to D1
    GPS_Driver(uint8_t rxPin = 4, uint8_t txPin = 5);
    
    bool begin(uint32_t baudRate = 9600);
    
    //update gps data
    void update();

    bool hasFix();
    
    //Data
    double getLatitude(); //Latitude in decimal degrees
    double getLongitude(); //Longitude in decimal degrees
    float getAltitude(); //Altitude in meters (less acurrate than bmp280)
    float getSpeed();    //Speed in m/s
    uint8_t getSatellites(); //Number of sattelites in view
    float getHDOP();     //Precision (less is better)
    
    //Time
    void getTime(uint8_t& hour, uint8_t& minute, uint8_t& second);
    void getDate(uint8_t& day, uint8_t& month, uint16_t& year);
    
    //Info
    uint32_t getFixAge(); //Get milliseconds since last valid location update
    bool isConnected();   //check if sensor is working
    uint32_t getCharsProcessed(); //number of characters processed
    uint32_t getSentencesWithFix(); //number of sentences received

private:
    TinyGPSPlus gps;
    SoftwareSerial* gpsSerial;
    
    uint8_t rxPin;
    uint8_t txPin;
    bool initialized;
};

#endif
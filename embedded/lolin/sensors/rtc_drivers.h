#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

class RTC_Driver {
public:
    RTC_Driver();
    
    bool begin();
    
    void setTimeFromCompile();

    void setTime(uint16_t year, uint8_t month, uint8_t day,
                 uint8_t hour, uint8_t minute, uint8_t second);
    

    //return Seconds since Jan 1, 1970
    uint32_t getUnixTime();
    
    //return Milliseconds since Jan 1, 1970
    uint64_t getMillis();
    
    //brief Get current time components
    void getTime(uint8_t& hour, uint8_t& minute, uint8_t& second);
    
    //get current date components
    void getDate(uint16_t& year, uint8_t& month, uint8_t& day);
    
    //Get RTC temperature (from internal sensor)
    float getTemperature();
    
    /**
     Check if RTC lost power since last use
     If true, time needs to be reset
     return true if power was lost
     */
    bool lostPower();
    

    //Check if sensor is responding, return true if RTC responds
    bool isConnected();
    
    
    //Time in format
    String getISO8601();

private:
    RTC_DS3231 rtc;
    bool initialized;
};

#endif
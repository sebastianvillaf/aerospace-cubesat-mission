
#include "rtc_driver.h"

RTC_Driver::RTC_Driver() : initialized(false) {   //sensor is off
}

bool RTC_Driver::begin() {                        //sensor is on
    Serial.print("Initializing RTC DS3231... ");
    
    if (!rtc.begin()) {
        Serial.println("FAILED!");
        initialized = false;
        return false;          //.ino gets this information
    }
    
    initialized = true;
    Serial.println("OK");
    
    //check if RTC lost power
    if (rtc.lostPower()) {
        Serial.println("[RTC] WARNING: RTC lost power");
        Serial.println("[RTC] Setting time from las compile..");
        setTimeFromCompile();
    }
    
    //Print current time
    DateTime now = rtc.now();
    Serial.print("[RTC] Current time: ");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
    return true;
}

void RTC_Driver::setTimeFromCompile() {
    if (!initialized) {         
        return;               //if sensor is off do anything
    }
    
    //Set time
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    
    Serial.println("[RTC] Time set from compilation timestamp");
}

void RTC_Driver::setTime(uint16_t year, uint8_t month, uint8_t day,
                        uint8_t hour, uint8_t minute, uint8_t second) {
    if (!initialized) {
        return;
    }
    
    DateTime dt(year, month, day, hour, minute, second);
    rtc.adjust(dt);
    
    Serial.println("[RTC] Time manually set");
}

uint32_t RTC_Driver::getUnixTime() {
    if (!initialized) {
        return 0;
    }
    
    DateTime now = rtc.now();
    return now.unixtime();
}


uint64_t RTC_Driver::getMillis() {
    if (!initialized) {
        return 0;
    }
    
    //Tomamos los segundos del RTC y los hacemos milisegundos (*1000)
    uint32_t unixTime = getUnixTime();
    uint64_t millis = (uint64_t)unixTime * 1000ULL;
    
    //add milliseconds
    millis += (::millis() % 1000);
    
    return millis;
}

//time
void RTC_Driver::getTime(uint8_t& hour, uint8_t& minute, uint8_t& second) {
    if (!initialized) {  //if sensor is off time is 0
        hour = 0;
        minute = 0;
        second = 0;
        return;
    }
    
    DateTime now = rtc.now();
    hour = now.hour();
    minute = now.minute();
    second = now.second();
}

//date
void RTC_Driver::getDate(uint16_t& year, uint8_t& month, uint8_t& day) {
    if (!initialized) {
        year = 0;
        month = 0;
        day = 0;
        return;
    }
    
    DateTime now = rtc.now();
    year = now.year();
    month = now.month();
    day = now.day();
}

//temperature
float RTC_Driver::getTemperature() {
    if (!initialized) {
        return 0.0;
    }
    
    return rtc.getTemperature();   //aditional temperature sensor from RTC
}


bool RTC_Driver::lostPower() {
    if (!initialized) {
        return true;   
    }
    
    return rtc.lostPower();    //.ino gets this information
}

bool RTC_Driver::isConnected() {
    return initialized;
}



//text format: "2026-02-06 15:00:00"
String RTC_Driver::getISO8601() {
    if (!initialized) {
        return "0000-00-00 00:00:00";
    }
    
    DateTime now = rtc.now();
    
    char buffer[20];
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",   //fill digits with 0 on the left
            now.year(), now.month(), now.day(),
            now.hour(), now.minute(), now.second());
    
    return String(buffer);
}
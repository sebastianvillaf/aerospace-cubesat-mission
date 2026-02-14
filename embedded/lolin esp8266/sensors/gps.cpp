include "gps_driver.h"

GPS_Driver::GPS_Driver(uint8_t rxPin, uint8_t txPin) 
    : rxPin(rxPin),
      txPin(txPin),
      initialized(false) {
    
    //SoftwareSerial for ESP8266
    gpsSerial = new SoftwareSerial(rxPin, txPin);
}

bool GPS_Driver::begin(uint32_t baudRate) {
    Serial.print("[GPS] Initializing on RX:");
    Serial.print(rxPin);
    Serial.print(" TX:");
    Serial.print(txPin);
    Serial.print(" @ ");
    Serial.print(baudRate);
    Serial.print(" baud");
    
    Serial.print(" (SoftwareSerial)... ");
    gpsSerial->begin(baudRate);
    
    initialized = true;
    Serial.println("OK");
    
    Serial.println("[GPS] Waiting for satellite fix...");
    Serial.println("[GPS] This may take 1-5 minutes outdoors");
    Serial.println("[GPS] GPS will NOT work indoors!");
    
    return true;
}

void GPS_Driver::update() {
    if (!initialized) {
        return;
    }
    
    //Feed the GPS analyzer with the serial data
    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        gps.encode(c);
    }
}

bool GPS_Driver::hasFix() {
    return gps.location.isValid();
}

double GPS_Driver::getLatitude() {
    if (!hasFix()) {
        return 0.0;
    }
    return gps.location.lat();
}

double GPS_Driver::getLongitude() {
    if (!hasFix()) {
        return 0.0;
    }
    return gps.location.lng();
}

float GPS_Driver::getAltitude() {
    if (!gps.altitude.isValid()) {
        return 0.0;
    }
    return gps.altitude.meters();
}

float GPS_Driver::getSpeed() {
    if (!gps.speed.isValid()) {
        return 0.0;
    }
    return gps.speed.mps();
}

uint8_t GPS_Driver::getSatellites() {
    if (!gps.satellites.isValid()) {
        return 0;
    }
    return gps.satellites.value();
}

float GPS_Driver::getHDOP() {
    if (!gps.hdop.isValid()) {
        return 9999.0;
    }
    return gps.hdop.hdop();
}

void GPS_Driver::getTime(uint8_t& hour, uint8_t& minute, uint8_t& second) {
    if (gps.time.isValid()) {
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();
    } else {
        hour = 0;
        minute = 0;
        second = 0;
    }
}

void GPS_Driver::getDate(uint8_t& day, uint8_t& month, uint16_t& year) {
    if (gps.date.isValid()) {
        day = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();
    } else {
        day = 0;
        month = 0;
        year = 0;
    }
}

uint32_t GPS_Driver::getFixAge() {
    return gps.location.age();
}

bool GPS_Driver::isConnected() {
    if (!initialized) {
        return false;
    }
    
    //Check if we are receiving data
    return gps.charsProcessed() > 10; 
}

uint32_t GPS_Driver::getCharsProcessed() {
    return gps.charsProcessed();
}

uint32_t GPS_Driver::getSentencesWithFix() {
    return gps.sentencesWithFix();
}
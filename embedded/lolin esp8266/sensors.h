#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>


struct SensorData {
    //TIMESTAMP 
    uint32_t timestamp_ms;          //From RTC or millis()
    uint32_t gps_time;              //GPS UTC time (if available)
    
    //BAROMETRIC SENSOR (BMP280)
    float pressure_hPa;             //Atmospheric pressure in hPa
    float temperature_C;            //Ambient temperature in Celsius
    float altitude_MSL;             //Altitude Mean Sea Level (raw)
    float altitude_AGL;             //Altitude Above Ground Level (calibrated!)
    bool bmp_valid;                 //True if BMP280 reading successful
    
    //INERTIAL MEASUREMENT UNIT (MPU6050)
    float pitch_deg;                //Pitch angle in degrees
    float roll_deg;                 //Roll angle in degrees
    float accel_x_g;                //X-axis acceleration in g
    float accel_y_g;                //Y-axis acceleration in g
    float accel_z_g;                //Z-axis acceleration in g
    bool imu_valid;                 //True if MPU6050 reading successful
    
    //GPS (NEO-6M)
    double latitude;                //Latitude in decimal degrees
    double longitude;               //Longitude in decimal degrees
    float gps_altitude_m;           //GPS altitude in meters
    float gps_speed_mps;            //Ground speed in m/s
    uint8_t satellites;             //Number of satellites in view
    bool gps_fix;                   //True if GPS has valid 3D fix
    
    //MISSION STATE
    float battery_voltage;          //Battery voltage in volts
    uint8_t mission_state_id;       //Current FSM state ID
    
    //ERROR FLAGS
    uint8_t error_flags;            //Bitfield of sensor errors
};

//Error flag bits
#define ERROR_BMP280_FAIL    (1 << 0)
#define ERROR_MPU6050_FAIL   (1 << 1)
#define ERROR_GPS_NO_FIX     (1 << 2)
#define ERROR_RTC_FAIL       (1 << 3)
#define ERROR_LOW_BATTERY    (1 << 4)
#define ERROR_SENSOR_TIMEOUT (1 << 5)


//SensorManager, manages all sensor hardware and provides unified interface
class SensorManager {
public:
    SensorManager();
    
    //Initialize all sensors, return true if critical sensors initialized successfully
    bool begin();
    
    //Perform ground calibration (altitude "tare"), num_samples is number of readings to average (20), return true if calibration successful
    bool calibrateGround(uint8_t num_samples = 20);
    
    /**
     Read all sensors and populate SensorData struct
     return true if at least one critical sensor read successfully
     */
    bool readAll(SensorData& data);
    
    float getGroundPressure() const;
    float getGroundAltitude() const;
    bool isCalibrated() const;
    void printStatus() const;

private:
    //Ground calibration values
    float groundPressure_hPa;
    float groundAltitude_MSL;
    bool calibrated;
    
    //Sensor health tracking
    bool bmp280_initialized;
    bool mpu6050_initialized;
    bool gps_initialized;
    bool rtc_initialized;
    
    //Battery monitoring
    const uint8_t BATTERY_PIN = 34;
    const float BATTERY_DIVIDER_RATIO = 2.0;
    const float LOW_BATTERY_THRESHOLD = 3.3;
    
    //Private methods
    bool readBMP280(SensorData& data);
    bool readMPU6050(SensorData& data);
    bool readGPS(SensorData& data);
    bool readRTC(SensorData& data);
    float readBatteryVoltage();
    void calculateOrientation(float ax, float ay, float az, 
                             float& pitch, float& roll);
};

#endif

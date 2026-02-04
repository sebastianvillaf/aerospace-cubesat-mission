/**
* @author Sebastian Villanueva Flores - Software Lead
 */

#ifndef FSM_H
#define FSM_H

#include <Arduino.h>


 //Each state has specific behaviors and data collection priorities

enum class MissionState {
    BOOT,              //System initialization, sensor checks
    IDLE,              //Pre-deployment, waiting for release
    ASCENT,            //Rising with drone (100m)
    DESCENT_FREE,      //Free fall before parachute deployment (~100m to ~80m)
    DESCENT_STABLE,    //Controlled descent with parachute (~80m to ~10m)
    LANDING,           //Final approach and touchdown (~10m to 0m)
    FINAL_REPORT,      //Post landing status transmission
    SAFE_MODE          //Error state, has to do minimal operations
};

/**
 * Responsibilities:
 * - Manage state transitions based on sensor data
 * - Trigger actions appropriate to current state
 * - Provide state information to telemetry system
 * - Handle error conditions gracefully
 */
class FSM {
public:
    
    //initializes FSM to BOOT state
     
    FSM();
    
    bool begin();

    /**
     * altitude_m Current altitude in meters (from BMP280 or GPS)
     * time_since_boot_ms time since system boot
     * gps_valid Whether GPS has valid fix
     */
    void update(float altitude_m, unsigned long time_since_boot_ms, bool gps_valid);

    
    //get mission state (const)
     
    MissionState getState() const;

    
    //get the current state as string (const)
    
    const char* getStateName() const;

    
    //get state as number ID (0-6), for compact telemetry packet because bandwith
    
    uint8_t getStateID() const;

    /**
     check if camera should capture image in current state (boolean)
     get true if conditions are right for image capture
     */
    bool shouldCaptureImage() const;

    /**
     check if telemetry should be transmitted (boolean)
     current_time_ms Current time in milliseconds
     get true if telemetry should be transmitted
     */
    bool shouldTransmitTelemetry(unsigned long current_time_ms);

    
    //force turn into SAFE_MODE (error handling)

    void enterSafeMode();


    //get time spent in current state (Duration in current state)
   
    unsigned long getTimeInState() const;

    
     //confirm that image has been captured (prevents bucle)
     //must be called after successful image capture
     
    void confirmImageCaptured();

private:
    MissionState currentState;           //current state
    MissionState previousState;          //previous state
    unsigned long stateEntryTime;        //millis() when current state was entered
    unsigned long lastTelemetryTime;     //millis() of last telemetry transmission

    //Altitude tracking for ASCENT → DESCENT_FREE transition
    float previousAltitude;              //last altitude reading (for detecting descent)
    float maxAltitudeReached;            //peak altitude during ASCENT state
    
    
    //these altitudes must be RELATIVE to ground (Above Ground Level)
    //main loop must implement AGL calibration during BOOT/IDLE
    const float LIFTOFF_DETECT_ALT = 10.0      //meters AGL (detect drone liftoff)
    const float PARACHUTE_DEPLOY_ALT = 80.0;   //meters AGL (not MSL! because margin of error)
    const float LANDING_DETECT_ALT = 10.0;     //meters AGL
    const float GROUND_LEVEL_ALT = 2.0;        //meters AGL (consider landed)
    const unsigned long TELEMETRY_INTERVAL = 1000;  //ms (1 Hz)
    const unsigned long IDLE_TIMEOUT = 300000; //5 min max in IDLE
    const float DESCENT_THRESHOLD = -0.5;      //m/s (negative = descending) "we are definitely falling"
    
    //Image capture control
    bool imageCaptured;                  //boolean
    const float IMAGE_CAPTURE_ALT = 40.0; //meters AGL (probably stable descent phase)
    
    /**
     Transition to new state
     newState Target state
     */
    void transitionTo(MissionState newState);
    
    
    //Entry actions when entering a state
    void onStateEntry();
    
    
     //Exit actions when leaving a state
    void onStateExit();
};

#endif
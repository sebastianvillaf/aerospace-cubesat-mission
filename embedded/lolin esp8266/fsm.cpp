/**
* @author Sebastian Villanueva Flores - Software Lead
 */

#include "fsm.h"


FSM::FSM() 
    : currentState(MissionState::BOOT),  
      previousState(MissionState::BOOT), //cubesat knows its in first state
      stateEntryTime(0),
      lastTelemetryTime(0),              
      imageCaptured(false) {             //cubesat knows it hasn't taken the image yet with esp32cam
}


bool FSM::begin() {
    stateEntryTime = millis();
    lastTelemetryTime = 0;
    imageCaptured = false;
    
    Serial.println("[FSM] Initialized in BOOT state");
    return true;
}


void FSM::update(float altitude_m, unsigned long time_since_boot_ms, bool gps_valid) {

    //Calculate altitude change rate
    float altitudeChange = altitude_m - previousAltitude;
    previousAltitude = altitude_m;

    if (altitude_m > maxAltitudeReached) {
        maxAltitudeReached = altitude_m;
    }
    

    switch (currentState) {
        
        case MissionState::BOOT:
            //transition to IDLE after initialization complete
            //check sensors
            if (time_since_boot_ms > 5000) {  //5 seconds for sensor stabilization
                transitionTo(MissionState::IDLE);  //turn into IDLE mission state (waiting)
            }
            break;                 //handbrake
            
        case MissionState::IDLE:
            //detect release by sudden altitude change or acceleration spike
            //altitude_m relative to ground (AGL)
            //Main loop calculate: altitude_AGL = current_altitude - ground_altitude
            //where ground_altitude is measured during BOOT (inicialization) using BMP280's sensor pressure averaging
            
            if (altitude_m > LIFTOFF_DETECT_ALT) {  //detected drone is lifting off
                transitionTo(MissionState::ASCENT);
            }
            
            //Safety timeout, prevent infinite IDLE bucle
            if (getTimeInState() > IDLE_TIMEOUT) {
                Serial.println("[FSM] WARNING: IDLE timeout exceeded");
                enterSafeMode();
            }
            break;
        
        case MissionState::ASCENDING:
            //stay in ASCENT until we detect DESCENT    
            if (altitudeChange < DESCENT_THRESHOLD) {
                Serial.print("[FSM] Descent detected! Rate: ");
                Serial.print(altitudeChange, 2);
                Serial.println("m/s");
                transitionTo(MissionState::DESCENT_FREE);
            }
            break;
        case MissionState::DESCENT_FREE:
            //Transition when parachute deploys (altitude drops below threshold)
            if (altitude_m < PARACHUTE_DEPLOY_ALT) {
                transitionTo(MissionState::DESCENT_STABLE);
            }
            
            //if cubesat is in free fall too long, assume parachute failed
            if (getTimeInState() > 10000) {  // 10 seconds max free fall
                Serial.println("[FSM] WARNING: Free fall timeout - parachute may have failed");
                //continue to DESCENT_STABLE anyway (data collection priority)
                transitionTo(MissionState::DESCENT_STABLE);
            }
            break;
            
        case MissionState::DESCENT_STABLE:
            //This is the CRITICAL PHASE for data collection
            //Transition to LANDING when close to ground
            if (altitude_m < LANDING_DETECT_ALT) {
                transitionTo(MissionState::LANDING);
            }
            break;
            
        case MissionState::LANDING:
            //Transition to FINAL_REPORT when on ground
            if (altitude_m < GROUND_LEVEL_ALT) {
                transitionTo(MissionState::FINAL_REPORT);
            }
            
            //Timeout: assume cubesat landed after 15 seconds in LANDING state
            if (getTimeInState() > 15000) {
                transitionTo(MissionState::FINAL_REPORT);
            }
            break;
            
        case MissionState::FINAL_REPORT:
            //Stay in this state indefinitely
            //Transmit final status periodically
            break;
            
        case MissionState::SAFE_MODE:
            //Minimal operations, no transitions out
            break;
    }
}

//Get current state
 
MissionState FSM::getState() const {
    return currentState;
}

//Get state name as string

const char* FSM::getStateName() const {
    switch (currentState) {
        case MissionState::BOOT:           return "BOOT";
        case MissionState::IDLE:           return "IDLE";
        case MissionState::DESCENT_FREE:   return "DESCENT_FREE";
        case MissionState::DESCENT_STABLE: return "DESCENT_STABLE";
        case MissionState::LANDING:        return "LANDING";
        case MissionState::FINAL_REPORT:   return "FINAL_REPORT";
        case MissionState::SAFE_MODE:      return "SAFE_MODE";
        default:                           return "UNKNOWN";
    }
}

//Get state as number ID (for compact telemetry)
uint8_t FSM::getStateID() const {
    return static_cast<uint8_t>(currentState);
}

//Check if image should be captured
bool FSM::shouldCaptureImage() const {
    //Only capture ONE image during stable descent at specific altitude
    if (currentState == MissionState::DESCENT_STABLE && !imageCaptured) {
        //This will be checked in main loop with altitude condition
        return true;
    }
    return false;
}

//Check if telemetry should be transmitted

bool FSM::shouldTransmitTelemetry(unsigned long current_time_ms) {
    //Don't transmit during BOOT
    if (currentState == MissionState::BOOT) {
        return false;
    }
    
    //Check if interval has passed
    if (current_time_ms - lastTelemetryTime >= TELEMETRY_INTERVAL) {
        lastTelemetryTime = current_time_ms;
        return true;
    }
    
    return false;
}

//Force SAFE_MODE (error handling)
void FSM::enterSafeMode() {
    Serial.println("[FSM] ENTERING SAFE MODE");
    transitionTo(MissionState::SAFE_MODE);
}


//Get time in current state
unsigned long FSM::getTimeInState() const {
    return millis() - stateEntryTime;
}

//Confirm image captured - CRITICAL for preventing infinite bucle
 
void FSM::confirmImageCaptured() {
    imageCaptured = true;
    Serial.println("[FSM] Image capture confirmed. Flag set to prevent bucle.");
}


//Transition to new state
void FSM::transitionTo(MissionState newState) {
    if (newState == currentState) {
        return;  //No transition needed
    }
    
    //Exit current state
    onStateExit();
    
    //Update state variables
    previousState = currentState;
    currentState = newState;
    stateEntryTime = millis();
    
    //Log transition
    Serial.print("[FSM] Transition: ");
    Serial.print(getStateName());
    Serial.print(" (");
    Serial.print(getTimeInState());
    Serial.println(" ms)");
    
    //Enter new state
    onStateEntry();
}


//Actions when entering a state
void FSM::onStateEntry() {
    Serial.print("[FSM] Entered state: ");
    Serial.println(getStateName());
    
    switch (currentState) {
        case MissionState::BOOT:
            Serial.println("[FSM] System booting...");
            break;
            
        case MissionState::IDLE:
            Serial.println("[FSM] Ready for deployment");
            break;
            
        case MissionState::DESCENT_FREE:
            Serial.println("[FSM] Free fall detected - awaiting parachute");
            break;
            
        case MissionState::DESCENT_STABLE:
            Serial.println("[FSM] Stable descent - PRIMARY DATA COLLECTION PHASE");
            //Reset image flag for this mission
            imageCaptured = false;
            break;
            
        case MissionState::LANDING:
            Serial.println("[FSM] Landing sequence initiated");
            //TODO: Trigger parachute release servo here
            break;
            
        case MissionState::FINAL_REPORT:
            Serial.println("[FSM] Mission complete - transmitting final report");
            break;
            
        case MissionState::SAFE_MODE:
            Serial.println("[FSM] SAFE MODE ACTIVE - Minimal operations only");
            break;
    }
}

//Actions when exiting a state
void FSM::onStateExit() {
    //Log time spent in state
    Serial.print("[FSM] Exiting ");
    Serial.print(getStateName());
    Serial.print(" after ");
    Serial.print(getTimeInState());
    Serial.println(" ms");
    
    //State-specific cleanup
    switch (currentState) {
        case MissionState::DESCENT_STABLE:
            if (!imageCaptured) {
                Serial.println("[FSM] WARNING: Exited DESCENT_STABLE without capturing image");
            }
            break;
            
        default:
            break;
    }

}

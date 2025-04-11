#include "MeMCore.h"
#include "Wire.h"

// Initialise electronics
MeDCMotor motor_L(M1);
MeDCMotor motor_R(M2);
MeLineFollower line_finder(PORT_2);
MeColorSensor colorsensor_L(PORT_1);
MeColorSensor colorsensor_R(PORT_4);

// Constant variables
const int mspeed = 100;
const int modulator = 5;

// Variables
int w_turn; // Where to turn -> 0: Left | 1 : Right
int mode = 0; // 0 : Line following | 1 : Arena

// Get new speed value with modifier
int set_speed(int modifier)
{
    // Max speed : 255
    int new_mspeed;

    if (mspeed + modulator >= 0 and mspeed + modulator <= 255) {
        // Use base motor speed and add modifier
        new_mspeed = mspeed + modifier;
    }
    else if (mspeed + modulator > 255) {
        new_mspeed = 255;
    }
    else {
        new_mspeed = 0;
    }

    return new_mspeed;
}

// Search line in turn when no line detected by sensors
void search_turn(int turn)
{
    // Variables
    int num = 0;
    int modifier = modulator;
    int sensor_state = line_finder.readSensors();
    
    // Search turn and add speed through time
    while (sensor_state == S1_OUT_S2_OUT) {
        if (turn == 0) {
            motor_L.run(set_speed(-modifier));
            motor_R.run(set_speed(modifier));
        }
        else {
            motor_L.run(set_speed(modifier));
            motor_R.run(set_speed(-modifier));
        }

        sensor_state = line_finder.readSensors();
        modifier = modifier + modulator;
        num ++;
        
        // Go back to first place
        if (num == 80) {
            num = 0;
            modifier = 0;
            
            while (num != 80) {
                if (turn == 0) {
                    motor_L.run(set_speed(modifier));
                    motor_R.run(set_speed(-modifier));
                }
                else {
                    motor_L.run(set_speed(-modifier));
                    motor_R.run(set_speed(modifier));
                }

                modifier = modifier + modulator;
                num ++;
            }

            break;
        }
    }
}

void setup()
{
    Serial.begin(115200);

    // Initialise color sensor
    colorsensor_L.SensorInit();
    colorsensor_R.SensorInit();
}

void loop()
{
    // Arena mode
    if (mode == 1) {
        motor_L.stop();
        motor_R.stop();
    }
    // Line mode
    else {
        int modifier;
        int sensor_state = line_finder.readSensors();

        switch (sensor_state)
        {
        case S1_IN_S2_IN:
            motor_L.run(-mspeed);
            motor_R.run(mspeed);
            break;
        
        case S1_IN_S2_OUT:
            // If two sensors turn left
            w_turn = 0;

            // Modifier speed right wheel
            modifier = modulator;
            
            // Add speed to right wheel to go back in track
            while (sensor_state == S1_IN_S2_OUT) {
                motor_L.run(-mspeed);
                motor_R.run(set_speed(modifier));
                modifier = modifier + modulator;
                sensor_state = line_finder.readSensors();
            }
            break;
        
        case S1_OUT_S2_IN:
            // If two sensors turn right
            w_turn = 1;

            // Modifier speed left wheel
            modifier = modulator;
                
            // Add speed to left wheel to go back in track
            while (sensor_state == S1_OUT_S2_IN) {
                motor_L.run(-set_speed(modifier));
                motor_R.run(mspeed);
                modifier = modifier + modulator;
                sensor_state = line_finder.readSensors();
            }
            break;
        
        case S1_OUT_S2_OUT:
            while (sensor_state == S1_OUT_S2_OUT) {
                if (w_turn == 0) {
                    search_turn(0);
                    search_turn(1);
                }
                else {
                    search_turn(1);
                    search_turn(0);
                }

                int num = 0;

                while (sensor_state == S1_OUT_S2_OUT) {
                    motor_L.run(-mspeed);
                    motor_R.run(mspeed);

                    sensor_state = line_finder.readSensors();
                    num ++;

                    if (num == 80) {
                        break;
                    }
                }

                sensor_state = line_finder.readSensors();
            }
            
            break;

        default:
            break;
        }
    }
}

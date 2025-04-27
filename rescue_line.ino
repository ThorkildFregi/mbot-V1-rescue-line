#include "MeMCore.h"
#include "Wire.h"

// Initialise electronics
MeDCMotor motor_L(M1);
MeDCMotor motor_R(M2);
MeLineFollower line_finder(PORT_2);
MeColorSensor colorsensor_L(PORT_1);
MeColorSensor colorsensor_R(PORT_4);

// Constant variables
const int mspeed = 255;
const int vspeed = 0.068;

// Variables
int w_turn; // Where to turn -> 0: Left | 1 : Right
int mode = 0; // 0 : Line following | 1 : Arena
uint8_t colorresult_L;
uint8_t colorresult_R;

// Move from a distance
int move(int distance)
{
    motor_L.run(-mspeed);
    motor_R.run(mspeed);
    delay(distance/vspeed);
}

// Search line in turn when no line detected by sensors
int search_turn(int turn)
{
    // Variables
    int num = 0;
    int sensor_state = line_finder.readSensors();
    
    // Search turn and add speed through time
    while (sensor_state == S1_OUT_S2_OUT) {
        if (turn == 0) {
            motor_L.run(mspeed);
            motor_R.run(mspeed);
        }
        else {
            motor_L.run(-mspeed);
            motor_R.run(-mspeed);
        }

        delay(10);

        sensor_state = line_finder.readSensors();
        num ++;
        
        // Go back to first place
        if (num == 80) {
            if (turn == 0) {
                motor_L.run(-mspeed);
                motor_R.run(-mspeed);
            }
            else {
                motor_L.run(mspeed);
                motor_R.run(mspeed);
            }

            delay(800);

            return 0;
        }
    }

    return 1;
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
        
            motor_L.run(-mspeed);
            motor_R.run(mspeed + 10);
            break;
        
        case S1_OUT_S2_IN:
            // If two sensors turn right
            w_turn = 1;
        
            motor_L.run(-mspeed - 10);
            motor_R.run(mspeed);
            break;
        
        case S1_OUT_S2_OUT:
            // Result of the searching
            int result;
            
            while (sensor_state == S1_OUT_S2_OUT) {
                if (w_turn == 0) {
                    result = search_turn(0); if (result == 1) {break;}
                    result = search_turn(1); if (result == 1) {break;}
                }
                else {
                    result = search_turn(1); if (result == 1) {break;}
                    result = search_turn(0); if (result == 1) {break;}
                }

                int num = 0;

                while (sensor_state == S1_OUT_S2_OUT) {
                    motor_L.run(-mspeed);
                    motor_R.run(mspeed);

                    sensor_state = line_finder.readSensors();
                    num ++;
                    
                    if (num == 10) {
                        colorresult_L = colorsensor_L.ColorIdentify();
                        colorresult_R = colorsensor_R.ColorIdentify();
                        if (colorresult_L == RED or colorresult_R == RED) {mode = 1; break;} // Active arena mode if red line detected
                    }
                    else if (num == 80) {
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

#include "MeMCore.h"
#include "Wire.h"

// Initialise electronics
MePort port(PORT_3);
MeDCMotor motor_L(M1);
MeDCMotor motor_R(M2);
MeLineFollower line_finder(PORT_2);
MeColorSensor colorsensor(PORT_1);
MeUltrasonicSensor ultraSensor(PORT_4);

Servo myservo1; // Create servo object
int16_t servo1pin =  port.pin1(); // Get pin servo

// Constant variables
const int mspeed = 100;
const float vspeed = 0.016;
const float aspeed = 0.135;

// Variables
int w_turn; // Where to turn -> 0: Left | 1 : Right
int mode = 1; // 0 : Line following | 1 : Arena
uint8_t colorresult;

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

int launch_ball()
{
    myservo1.write(30); // Pliers down
    delay(2000);
    
    motor_L.run(-mspeed);
    motor_R.run(mspeed);

    delay(4000);
    
    motor_L.run(-mspeed);
    motor_R.run(-mspeed);

    delay(500);

    motor_L.stop();
    motor_R.stop();

}

// Final stade actions
int arena_mode()
{
    int i = 0;
    float distance;
    while (i != 20) {
        // Get distance to the object in front of the bot
        distance = ultraSensor.distanceCm();
        
        motor_L.run(mspeed);
        motor_R.run(mspeed);

        delay(80);

        motor_L.stop();
        motor_R.stop();

        i++;
        // If the difference between the distance detected is too big then
        if (abs(distance - ultraSensor.distanceCm()) >= 5.0) {
            launch_ball();
            while(1);
        }
    }
}

void setup()
{
    Serial.begin(115200);

    // Attach pin to servo object and make servo at 0°
    myservo1.attach(servo1pin);
    
    myservo1.write(0); // Pliers up
    delay(2000);

    // Initialise color sensor
    colorsensor.SensorInit();
}

void loop()
{
    // Arena mode
    if (mode == 1) {
        arena_mode();

        while(1);
    }
    // Line mode
    else {
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

            motor_L.stop();
            motor_R.stop();
            colorresult = colorsensor.ColorIdentify();
            if (colorresult == RED) {mode = 1; break;} // Active arena mode if red line detected

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
                    if (num == 0) {
                        motor_L.stop();
                        motor_R.stop();
                        colorresult = colorsensor.ColorIdentify();
                        if (colorresult == RED) {mode = 1; break;} // Active arena mode if red line detected
                    }
                    motor_L.run(-mspeed);
                    motor_R.run(mspeed);

                    sensor_state = line_finder.readSensors();
                    num ++;

                    delay(10);
                    
                    if (num == 40) {
                        break;
                    }
                }

                if (mode == 1) {break;}

                sensor_state = line_finder.readSensors();
            }
            
            break;

        default:
            break;
        }
    }
}

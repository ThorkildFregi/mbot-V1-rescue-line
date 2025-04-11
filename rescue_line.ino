#include "MeMCore.h"
#include "Wire.h"

// Initialisation
MeDCMotor motor_L(M1);
MeDCMotor motor_R(M2);
MeLineFollower lineFinder(PORT_1);
MeColorSensor colorsensor_L(PORT_2);
MeColorSensor colorsensor_R(PORT_4);

// Tiles settings
const int unknown_tile = 0;
const int forbidden_tile = 1;
const int robot_position = 2;
const int sensor_position = 3;
const int path_tile = 4;
const int no_path_tile = 5;

int 

void setup()
{
  Serial.begin(115200);

  colorsensor_L.SensorInit();
  colorsensor_R.SensorInit();
}

void loop()
{

}

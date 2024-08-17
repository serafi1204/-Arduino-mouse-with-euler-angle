/* Main device
* 현재 문제점
* 1. 스크롤 장치 반영 안함. 임시로 SW3, 4로 사용 중.
*   SW3 == 1 -> up scroll
*   SW4 == 1 -> down scroll 
*/

#include <SPI.h>
#include <Arduino_LSM9DS1.h>
#include <math.h>
extern "C"{
  #include <rtwtypes.h>
  #include <kal_tool_types.h>
  #include <rt_nonfinite.h>
  #include <kal_tool.h>
  #include <kal_tool_terminate.h>
  #include <kal_tool_initialize.h>
  #include <trajectory_tool.h>
  #include <trajectory_tool_types.h>
  #include <trajectory_tool_terminate.h>
  #include <trajectory_tool_initialize.h>
  #include <getQuaternion.h>
}

// I2C Setup 
#define LSM9DS1_M 0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // Would be 0x6A if SDO_AG is LOW

// Output Settings
#define PRINT_CALCULATED
#define PRINT_SPEED 1 // 20 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time
double g_xyz[3];
double a_xyz[3];
double m_xyz[3];
double acc_tmp[6];
double ang_vel_tmp[6];
double prev_position_tmp[3];
double prev_velocity_tmp[3];
c_matlabshared_rotations_intern prev_orientation = {1, 0, 0, 0};
d_matlabshared_rotations_intern orient;
const double g = 9.80;
const double acc_mag_threshold = 0.3;
double grav[3]; 
int fs = 50;

static double orientationEuler_kal[3];
double orientationEuler_kal_tmp[3];
double orientationEuler_kal_d[3];

//temp var for replacing library
float t0, t1, t2;

// Switchs
#define SW0 2 // mouse left switch
#define SW1 3 // mouse right switch
#define SW2 4 // disable mouse
#define SW3 5 //scroll up
#define SW4 6 // scroll down

int sw = 0;

void setup() 
{
  
  Serial.begin(9600);
  kal_tool_initialize();
  trajectory_tool_initialize();

  if (!IMU.begin())
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                  "work for an out of the box LSM9DS1 " \
                  "Breakout, but may need to be modified " \
                  "if the board jumpers are.");
    while (1)
      ;
  }
  // Get initial acceleration 
  IMU.readAcceleration(t0, t1, t2);
  acc_tmp[0] = (double)t0;
  acc_tmp[2] = (double)t1;
  acc_tmp[4] = (double)t2;

  // Initialize global variables
  prev_position_tmp[0] = 0;
  prev_position_tmp[1] = 0;
  prev_position_tmp[2] = 0;
  prev_velocity_tmp[0] = 0;
  prev_velocity_tmp[1] = 0;
  prev_velocity_tmp[2] = 0;
  ang_vel_tmp[0] = 0;
  ang_vel_tmp[2] = 0;
  ang_vel_tmp[4] = 0;
  grav[0] = 0;
  grav[1] = 0;
  grav[2] = 0;
  getQuaternion(0, 0, 0, &prev_orientation);

  // Switch
  pinMode(SW0, INPUT_PULLUP);
  pinMode(SW1, INPUT_PULLUP); 
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP); 
}

// Constantly read from the IMU and calculate orientation and position
void loop()
{  
  if ((lastPrint + PRINT_SPEED) < millis())
  {
    IMU.readGyroscope(t0, t1, t2);
    g_xyz[0] = (double)t0;
    g_xyz[1] = (double)t1;
    g_xyz[2] = (double)t2;  
    IMU.readAcceleration(t0, t1, t2);
    a_xyz[0] = (double)t0;
    a_xyz[1] = (double)t1;
    a_xyz[2] = (double)t2;  
    IMU.readMagneticField(t0, t1, t2);
    m_xyz[0] = (double)t0;
    m_xyz[1] = (double)t1;
    m_xyz[2] = (double)t2;
    // Scale gyroscope, accelerometer, and magnetometer values to the
    // units needed by algorithms
    
    // Gyroscope from degrees to radians
    g_xyz[0] = g_xyz[0] * 0.0174533;
    g_xyz[1] = g_xyz[1] * 0.0174533;
    g_xyz[2] = g_xyz[2] * 0.0174533;
    // Acceleration from g to m/s^2
    a_xyz[0] = a_xyz[0] * g;
    a_xyz[1] = a_xyz[1] * g;
    a_xyz[2] = a_xyz[2] * g;
    // Magnetometer from gauss to microTeslas
    m_xyz[0] = m_xyz[0] * 100;
    m_xyz[1] = m_xyz[1] * 100;
    m_xyz[2] = m_xyz[2] * 100; 
    
    // Call the kalman filter algorithm and trajectory calculation
    main_kal_tool(a_xyz, g_xyz, m_xyz, acc_tmp, ang_vel_tmp, prev_position_tmp, prev_velocity_tmp, prev_orientation, orient);    
    lastPrint = millis(); // Update lastPrint time

    prt();
  }
}

#define TH 60
#define GAIN 2 // TH*GAIN ~ 127
char d2c(double v) {
  if (v > TH) v = TH;
  if (v < -TH) v = -TH;
  
  int iv = (int)(v*GAIN);
  if (iv == 10) iv = 9;

  return (char)iv;
}

void prt() {
  // get d/dt
  for (int i = 0; i<3; i++) {
    orientationEuler_kal_d[i] = orientationEuler_kal[i] - orientationEuler_kal_tmp[i];
    orientationEuler_kal_tmp[i] = orientationEuler_kal[i];
  }

  // get switch
  sw = 0;
  for (int i=SW0; i<=SW4; i++) {
    sw += digitalRead(i) == true ? 0 : 1<<(i-SW0);
  }

  // Print the orientation and position to serial.
  Serial.print(d2c(-orientationEuler_kal_d[0]));
  Serial.print(d2c(orientationEuler_kal_d[1]));
  Serial.println(d2c(sw));
}

// Obtain orientation and position using
static void main_kal_tool(const double acc[3], 
        const double gyr[3], const double mg[3], double acc_tmp[6], double ang_vel_tmp[6], 
        double prev_position_tmp[3], double prev_velocity_tmp[3], 
        c_matlabshared_rotations_intern prev_orientation, 
        d_matlabshared_rotations_intern orient) {

  static double angularvelocity[3];
  static double pos[3];
  static double vel[3];
  static double cur_position[3];
  static double velocity[6];
  static double acceleration[6];
  static double angularVelocity[6];
  static double acc_filtered[3];
  float acc_magnitude;

  // Call the Kalman filter to obtain orientation and angular velocity
  // Inputs: acceleration, angular velocity, magnetic field strength
  // Outputs: orientation (in NED coordinates), angular velocity
  kal_tool(acc, gyr, mg, orientationEuler_kal, angularvelocity);
  // Convert the orientation in NED coordinates into orientation in quaternions
  // Inputs: Yaw, pitch, roll
  // Outputs: Orientation in quaternions
  getQuaternion(orientationEuler_kal[0], orientationEuler_kal[1], orientationEuler_kal[2], &prev_orientation);
}
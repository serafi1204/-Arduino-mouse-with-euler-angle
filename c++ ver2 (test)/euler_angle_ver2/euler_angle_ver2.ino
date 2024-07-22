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

//////////////////////////
// LSM9DS1 Library Init //
//////////////////////////
// kill : LSM9DS1 imu;

// I2C Setup 
// SDO_XM and SDO_G are both pulled high, so our addresses are:
#define LSM9DS1_M 0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // Would be 0x6A if SDO_AG is LOW

// Output Settings
#define PRINT_CALCULATED
#define PRINT_SPEED 5 // 20 ms between prints
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

// printing
double prev_angle[3] = {0,};
int mx[3] = {0,};
int mn[3] = {0,};

//temp var for replacing library

float t0, t1, t2;

void readAndfloat2double(int (*pf)(float&, float&, float&), double *a, double *b, double *c) {
  (*pf)(t0, t1, t2);
  *a = double(t0);
  *b = double(t1);
  *c = double(t2);
}

#define SIZE 10
double m[3][SIZE] = {{0,},};

double avg(double *map, double v) {
  double acc = 0;

  acc += v;
  for (int i=SIZE-1; i>0; i--) {
    map[i] = map[i-1];
    acc += map[i];
  }
  map[0] = v;

  return acc/SIZE;
}

void setup() 
{

  for (int i=0; i<3; i++) {
    mx[i] = 127;
    mn[i] = 127;
  }
  
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
  }

  prt();
}

void prt() {
  double angle[3] = {0,};
  char cangle[3];
  int iangle[3];

  for (int i=0; i<3; i++) {
    double tmp = avg(m[i], orientationEuler_kal[i]);
    angle[i] = tmp - prev_angle[i];
    iangle[i] = int(angle[i]+128);
    cangle[i] = char(iangle[i]);

    if (cangle[i] == char(10)) cangle[i] == char(11);
    if (cangle[i] == char(00)) cangle[i] == char(01);

    prev_angle[i] = tmp;
  }

  Serial.print("\n");
  Serial.print(cangle[0]); //Serial.print(char(0));
  Serial.print(cangle[1]); //Serial.print(char(0));
  Serial.print(cangle[2]); //Serial.print(char(0));
  
  /*
  Serial.print("\nint : ");
  for (int i=0; i<3; i++) {
    Serial.print(iangle[i]-128);
    Serial.print('\t');
  }

  Serial.print("\ndouble : ");
  for (int i=0; i<3; i++) {
    Serial.print(angle[i]);
    Serial.print('\t');
  }
  */
  /*
  for (int i=0; i<3; i++) {
    Serial.print(int(angle[i]+128));
    Serial.print(",");
  }
  Serial.print("\n");

  for (int i=0; i<3; i++) {
    Serial.print(angle[i]);
    Serial.print(",");
  }
  Serial.print("\n");

  Serial.print("max: ");
  for (int i=0; i<3; i++) {
    Serial.print(mx[i]);
    Serial.print(",");
  }
  Serial.print("\n");
  Serial.print("min: ");
  for (int i=0; i<3; i++) {
    Serial.print(mn[i]);
    Serial.print(",");
  }
  Serial.print("\n");
  */
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

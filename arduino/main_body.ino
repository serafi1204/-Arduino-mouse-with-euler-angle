#include <Wire.h>
#include <SPI.h>
#include <Arduino_LSM9DS1.h>
#include <math.h>

// Loop frequancy control
#define SERIAL_SPEED 9600
#define SPEED 1
// filtering
#define FILTER_SIZE 5
// Switchs
#define SW0 2 // mouse left switch
#define SW1 3 // mouse right switch
#define SW2 4 // disable mouse
#define SW3 5 //scroll up
#define SW4 6 // scroll down
// Print 
#define TH 4
#define CHAR_MAX 125

// Loop frequancy control
static unsigned long lastPrint = 0; 

// IMU
double gyo_init[3] = {0,};
float f_tmp[3]; // tmp space
double gyo[3]; // 119.00 Hz
//double acc[3]; // 119.00 Hz
//double mag[3]; // 119.00 Hz
int readSense(); // read values

// filtering
int idx = 0;
double gyo_acc[3]; // 119.00 Hz, 8.4ms
//double acc_acc[3]; // 119.00 Hz, 8.4ms
//double mag_acc[3]; // 20.00 Hz, 50ms
double gyo_stack[3][FILTER_SIZE];
//double acc_stack[3][FILTER_SIZE];
//double mag_stack[3][FILTER_SIZE];
void avgFilter(double* v, double filter[][FILTER_SIZE], double* acc);

// Switchs
int sw = 0;

// Print 
int gain = (int)(CHAR_MAX/TH);
char d2c(double v); // double to char
void prt();


// main
void setup() {
  // Serial comm
  Serial.begin(SERIAL_SPEED);

  // Start IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // Switch
  pinMode(SW0, INPUT_PULLUP);
  pinMode(SW1, INPUT_PULLUP); 
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP); 
}

void loop() {
  if ((lastPrint + SPEED) >= millis()) return;
  if (readSense() == 1) return;

  // Converse unit
  for (int i=0; i<3; i++) {
    gyo[i] *= 0.0174533;
    //acc[i] *= 9.80665;
    //mag[i] *= 100;
  }

  // filter : smoothing
  avgFilter(gyo, gyo_stack, gyo_acc);

  // print
  prt();

  // end line
  Serial.print("\n");
  // reset timer
  lastPrint = millis();
  // next index
  idx++;
  if (idx == FILTER_SIZE) idx = 0;
}

//functions;
// IMU
int readSense() {
  if (!IMU.gyroscopeAvailable() || !IMU.accelerationAvailable() || !IMU.magneticFieldAvailable()) return 1;

  IMU.readGyroscope(f_tmp[0], f_tmp[1], f_tmp[2]);
  for (int i=0; i<3; i++) gyo[i] = (double)f_tmp[i];

  /*
  IMU.readAcceleration(f_tmp[0], f_tmp[1], f_tmp[2]);
  for (int i=0; i<3; i++) acc[i] = (double)f_tmp[i];
  
  IMU.readMagneticField(f_tmp[0], f_tmp[1], f_tmp[2]);
  for (int i=0; i<3; i++) mag[i] = (double)f_tmp[i];
  */
  return 0;
}

// filtering
void avgFilter(double* v, double filter[][FILTER_SIZE], double* acc) {
  for (int i=0; i<3; i++) {
    int sum = 0;
    
    acc[i] += v[i]-filter[i][idx]; 
    filter[i][idx] = v[i];

    v[i] = acc[i]/FILTER_SIZE;
  }
}

// Print 
char d2c(double v) {
  if (v > TH) v = TH;
  if (v < -TH) v = -TH;
  
  int iv = (int)(v*gain);
  if (iv == 10) iv = 9;

  return (char)iv;
}

void prt() {
  sw = 0;
  for (int i=SW0; i<=SW4; i++) {
    sw += digitalRead(i) == true ? 0 : 1<<(i-SW0);
  }

  // Print the orientation and position to serial.
  Serial.print(d2c(gyo[2]));
  Serial.print(d2c(gyo[1]));
  Serial.print((char)sw);
}

void debug(double* list) {
  for (int i=0; i<3; i++) {
    Serial.print(list[i]);
    Serial.print(" ");
  }
}

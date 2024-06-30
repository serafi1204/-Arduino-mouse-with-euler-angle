#include <Arduino_LSM9DS1.h> // Arduino Nano 33 BLE (또는 BLE Sense)의 LSM9DS1 센서 헤더파일
// 위의 헤더파일은 상단의 "툴"에서 "라이브러리 관리..."를 누른 후 LSM9DS1을 검색하면 나온다(by Arduino)

const int dt = 10; // 센서값 측정 시간 간격, 단위 = ms
const int cal = 50; // bias 구할 때 몇번 측정해서 평균 낼 것인지
const float DTOR = PI / 180; // 각도를 라디안으로

float Roll = 0, Pitch = 0, Yaw = 0; // 각 축 회전각 저장할 변수
int t = 0; // 시간을 저장할 변수
int GxBias = 0, GyBias = 0, GzBias = 0; // 각 축 각속도 bias, 단위 = dps

void setup() {
  Serial.begin(9600); // Arduino IDE의 시리얼 모니터와 통신, 9600 보드레이트
  while (!Serial); // 시리얼 모니터를 열은 후부터 시작
  
  if (!IMU.begin()) { // 그럴 일은 없지만, 센서를 찾지 못했을 경우
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  for(int ii = 0; ii < cal; ii++){
    float Gx, Gy, Gz; // 센서 측정값을 저장할 변수
    IMU.readGyroscope(Gx, Gy, Gz); // 센서 측정값 불러오기

    // NED 좌표에 맞춰주기
    Gx = -Gx;
    Gy = -Gy;
    Gz = -Gz;

    // 측정값을 변수에 더해서 저장
    GxBias += Gx;
    GyBias += Gy;
    GzBias += Gz; 
    
    delay(100); // 시간 간격
  }
  // 측정값 평균내기
  GxBias /= cal;
  GyBias /= cal;
  GzBias /= cal; 
  
  Serial.println("Time(s) Roll(deg) Pitch(deg) Yaw(deg)"); // 측정값: 시간(초), 각 축의 오일러 각(도)
}

float mod(float a, float b) {
  if (a >= 0) {
    while (a >= b) a -= b;
  } else {
    while (-a <= -b) a += b;
  }

  return a;
}

void loop() {
  float Gx, Gy, Gz; // 센서 측정값을 저장할 변수
  float dRoll, dPitch, dYaw; // 오일러 각의 ﻿시간에 따른 변화율

  IMU.readGyroscope(Gx, Gy, Gz); // 센서 측정값 불러오기

  // NED 좌표에 맞춰주기
  Gx = -Gx;
  Gy = -Gy;
  Gz = -Gz;
  
  // Bias 제거
  Gx -= GxBias;
  Gy -= GyBias;
  Gz -= GzBias;

  // 각속도계를 오일러 각으로
  dRoll = Gx + sin(DTOR * Roll) * tan(DTOR * Pitch) * Gy + cos(DTOR * Roll) * tan(DTOR * Pitch) * Gz;
  dPitch = cos(DTOR * Roll) * Gy - sin(DTOR * Roll) * Gz;
  dYaw = sin(DTOR * Roll) / cos(DTOR * Pitch) * Gy + cos(DTOR * Roll) / cos(DTOR * Pitch) * Gz;

  dRoll = dRoll * dt * 0.001;
  dPitch = dPitch * dt * 0.001;
  dYaw = dYaw * dt * 0.001;

  Roll += dRoll;
  Pitch += dPitch;
  Yaw += dYaw;
  
  //Serial.print(dt * 0.001 * t++); // 시간 출력
  //Serial.print(" ");
  Serial.print(dRoll); // Roll 출력
  Serial.print(" ");
  Serial.print(dPitch); // Pitch 출력
  Serial.print(" ");
  Serial.println(dYaw); // Yaw 출력
  delay(dt); // 센서값 측정시간 간격
}
#include <iostream>
#include "SerialClass.h"
#include <string>
#include <vector>
#include <sstream>
#include <Windows.h>
#include <time.h>

using namespace std;

//#define DEBUG 

time_t s, e;

// Arduino
#define PORT "COM8"
#define PERIOD 5

Serial* SP = new Serial("COM8");
double YPR[3] = {0,};
double preYPR[3] = {0,};
double dYPR[3] = {0,};

// Mouse
#define TRESHOLD 2
#define GAIN 1

double tp[2] = {0,};
double dp[2] = {0,};

POINT p;


void getYPR(double *ypr) {
    char c;
    char line[3];

    while (c != '\n') {
        SP->ReadData(&c, 1);
    }

    SP->ReadData(line, 3);

    for (int i=0; i<3; i++) {
        ypr[i] = (double)(0x00FF & line[i])-128; 
    }
}

int main() {
    while (true) {
        char c;
        SP->ReadData(&c, 1);

        if (c == '\n') {
            cout << "\n";
            continue;
        }
        cout << (double)(0x00FF & c)-128 << 0; 

        Sleep(0.2);
    }

    while (true) {
        while (SP->IsConnected()) {
            Sleep(1);

            // get data
            getYPR(dYPR);

            // make x, y
            tp[0] = dYPR[0];
            tp[1] = dYPR[1];
            for(int i = 0; i<2; i++) {
                if (-TRESHOLD < tp[i] && tp[i] < TRESHOLD) {
                    dp[i] = 0;
                    continue;
                }

                dp[i] = tp[i]*GAIN*PERIOD;
            }
            

            // control mouse
            if(GetCursorPos(&p)) {
                SetCursorPos(p.x+dp[0], p.y+dp[1]);
            }
        
            #ifdef DEBUG
            cout << "tx: " << tp[0] << "/ty: " << tp[1] << "\n";
            cout << "dx: " << dp[0] << "/dy: " << dp[1] << "\n";
            #endif
        }
    }
    cout << "Fail to connect." << "\n";
}

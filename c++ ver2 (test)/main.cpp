#include <iostream>
#include "SerialClass.h"
#include <string>
#include <vector>
#include <sstream>
#include <Windows.h>
#include <time.h>

using namespace std;

#define DEBUG 

time_t s, e;
double period = 0;

// Arduino
#define BAUDRATE 115200
#define PORT "COM8"

Serial* SP = new Serial("COM8");
double YPR[3] = {0,};
double preYPR[3] = {0,};
double dYPR[3] = {0,};

// Mouse
#define TRESHOLD 1
#define GAIN 10

double tp[2] = {0,};
double dp[2] = {0,};

POINT p;


void getYPR(double *ypr) {
    int cnt = 0;
    char c;
    string line = "";

    while (cnt != 2) {
        SP->ReadData(&c, 1);

        if (cnt == 1) line.push_back(c);
        if (c == '\n') cnt++;
    }

    stringstream ss(line);
    string word;
    int i = 0;
    while (getline(ss, word, ',')){
        ypr[i] = stod(word);
        i++;
    }
}

int main() {
    s = clock(); 
    while (true) {
        while (SP->IsConnected()) {

            // get data
            getYPR(YPR);
            // make x, y
            for(int i = 0; i<2; i++) {
                dYPR[i] = YPR[i] - preYPR[i];
                preYPR[i] = YPR[i];
            }

            // make x, y
            tp[0] = dYPR[0];
            tp[1] = dYPR[1];
            for(int i = 0; i<2; i++) {
                dp[i] = tp[i]*GAIN*period;
            }

            // control mouse
            if(GetCursorPos(&p)) {
                SetCursorPos(p.x+dp[0], p.y+dp[1]);
            }

            e = clock();
            period = (double)(e-s);
            s = e;
            
            #ifndef DEGUG
            cout << "tx: " << tp[0] << "/ty: " << tp[1] << "\n";
            cout << "dx: " << dp[0] << "/dy: " << dp[1] << "\n";
            cout << "period: " << period << " ms\n";
            #endif
        }
    }
    cout << "Fail to connect." << "\n";
}
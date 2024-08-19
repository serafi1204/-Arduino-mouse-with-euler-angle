/* Ver. 3.0.0
* 현재 문제
*/

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <Windows.h>
#include <time.h>
#include "SerialClass.h"
#include "readline.h"
#include "filter.h"
using namespace std;

/********************************************** Freq */
#define SPEED 5

clock_t s = clock();
clock_t e = clock();

/********************************************** For debugging
* "#define DEBUG"를 주석처리하면 디버깅 내용이 출력되지 않습니다.
* 특정 기능 ENABLE을 주석처리하면 해당 기능은 동작하지 않습니다.
*/
#define DEBUG
#define ENABLE_POSITION 
#define ENABLE_CLICK
#define ENABLE_SCROLL 
#ifdef DEBUG
void prt(bool cond, string name, int v) {
	cout << "\t" << name << "\t: ";
	if (cond) cout << v;
	else cout << "Disconneted.";
	cout << "          \n";
}
#endif

/********************************************** Main Arduino
* IMU 포함 메인 디바이스
* line = {dx, dy, click}
* click = {left click, right click, mouse disable}
*/
#define LEN_MAIN 3

void filter_main(int* line, string stack); // 데이터 후처리 + 필터
READLINE ardu_main("COM8", 9600, LEN_MAIN, filter_main);
int line_main[LEN_MAIN];
int enable = false;

/********************************************** Sub Arduino
* 클릭 전용 보조 디바이스
* line = {click}
* click = {left click, right click, enable, scroll up, scroll down}
*/
#define LEN_SUB 1

void filter_sub(int* line, string stack); // 데이터 후처리 + 필터
READLINE ardu_sub("COM4", 9600, LEN_SUB, filter_sub);
int line_sub[LEN_SUB];

// Mouse Controll
#define WHEEL_GAIN 10

POINT p, dp;
int click = 0; // 2bit int, {right, left}
int click_main = 0;
int click_sub = 0;
int click_his[1] = { 0, };
int scroll = 0; // -1: down, 0: stop, 1: up


/********************************************** Main */
int main() {
	while (true) {
		if (!ardu_main.isConnected()) ardu_main.tryConnect();

		// Debugging
		#ifdef DEBUG
			bool main_body = ardu_main.isConnected();
			bool sub_body = ardu_sub.isConnected();

			COORD pos = {0, 3};
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
			
			cout << "[Mouse]\n";
			prt(true, "Enable", enable);
			prt(true, "dx", dp.x);
			prt(true, "dy", dp.y);
			prt(true, "left", click & 0b01);
			prt(true, "right", click & 0b10);
			prt(true, "scroll", scroll);

			cout << "[Main]\n";
			prt(main_body, "dx", line_main[0]);
			prt(main_body, "dy", line_main[1]);
			prt(main_body, "click", line_main[2]);
			
			cout << "[Sub]\n";
			prt(sub_body, "click", line_sub[0]);
		#endif

		// Serial communication
		scroll = 0;
		if (ardu_sub.readline(line_sub)) {
			click_sub = line_sub[0] & 0b11;
		}
		if (ardu_main.readline(line_main)) {
			enable = ardu_main.enable;
			dp.x = line_main[0];
			dp.y = line_main[1];

			line_main[2] >>= 0;
			click_main = line_main[2] & 0b11;
			enable &= ~((line_main[2] >> 2) & 0b1);
			scroll = (line_main[2] & 0b001000) ? 1 : 0;
			scroll += (line_main[2] & 0b10000) ? -1 : 0;
		}

		click_his[0] = click;
		click = click_sub | click_main;

		// Contoll mouse
		e = clock();
		if ((double)(e - s) < SPEED) continue;
		s = e;

		if (!enable) { continue; }
		if (!GetCursorPos(&p)) { continue; }

		// position
		#ifdef ENABLE_POSITION
			p.x += dp.x;
			p.y += dp.y;
			SetCursorPos(p.x, p.y);
		#endif

		// click
		#ifdef ENABLE_CLICK
			if ((click & 0b01) != (click_his[0] & 0b01)) {
				if (click & 0b01) {
					mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE, p.x, p.y, 0, ::GetMessageExtraInfo());
				}
				else {
					mouse_event(MOUSEEVENTF_LEFTUP | MOUSEEVENTF_ABSOLUTE, p.x, p.y, 0, ::GetMessageExtraInfo());
				}
			}
			if ((click & 0b10) != (click_his[0] & 0b10)) {
				if (click & 0b10) {
					mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_ABSOLUTE, p.x, p.y, 0, ::GetMessageExtraInfo());
				}
				else {
					mouse_event(MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_ABSOLUTE, p.x, p.y, 0, ::GetMessageExtraInfo());
				}
			}
		#endif

		// Scroll
		#ifdef ENABLE_SCROLL
			if (scroll == 1) {
				mouse_event(MOUSEEVENTF_WHEEL | MOUSEEVENTF_ABSOLUTE, p.x, p.y, WHEEL_GAIN, 0);
			} else if (scroll == -1) {
				mouse_event(MOUSEEVENTF_WHEEL | MOUSEEVENTF_ABSOLUTE, p.x, p.y, -WHEEL_GAIN, 0);
			}
		#endif
	}
}

/********************************************** Filter 
* 필터의 인자는 수정되어선 안된다.
* line에 결과를 저장한다.
* stack에 Serial를 통해 받은 char가 들어있다.
*/

FILTER aTrimmedMean(9, 0);

void filter_main(int* line, string stack) { 
	// Char to integer
	for (int i = 0; i < LEN_MAIN; i++) {
		line[i] = (int)(signed char)stack[i];
	}

	for (int i = 0; i < 2; i++) {
		line[i] = -line[i];
		line[i] /= 2;
		//line[i] = aTrimmedMean.filtering(line[i]);
	}

	// if you add filter, add here.
}

void filter_sub(int* line, string stack) { 
	// Char to integer
	line[0] = (int)(signed char)stack[0] - 48; 

	// if you add filter, add here.
}

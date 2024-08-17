#ifndef READLINE_H
#define READLINE_H

#include <iostream>
#include <string>
#include "SerialClass.h"
using namespace std;


class READLINE {
private:
	// Serial port
	Serial* SP;
	string COM = "COM1";
	int BAUDRATE = 9600;
	// Comm value
	int lineLen = 1;
	int cnt = 0;
	string stack = "";
	void (*getData)(int*, string);


public:
	// initalization
	READLINE(string COM, int BAUDRATE, int lineLen, void (*getData)(int*, string)) : COM(COM), BAUDRATE(BAUDRATE), lineLen(lineLen), getData(getData) {
		SP = new Serial(COM.c_str(), BAUDRATE);
		for (int i = 0; i < lineLen; i++) stack.append(" ");
	}
	
	// values
	bool enable = false;
	bool readline(int* line);
	bool isConnected();
};

/*
Serial port���� char 1���� �д´�.
'\n'�� �Է��� �� ������ ���� lineLen�� ���ٸ� ������ line���� �ν��Ѵ�.
Line�� �νĵ� ��� getData �Լ��� ����ȴ�.

return: {line �ν� ����}
int* line: line�� �νĵ� ��� data ��ȯ.
*/
bool READLINE::readline(int* line) {
	char c;

	this->enable = false;

	if (!(SP->IsConnected())) return this->enable;

	this->SP->ReadData(&c, 1);

	if (c == '\n' || c == 13 || c == -52) {
		// check get all data
		if (cnt == lineLen) {
			this->enable = true;
			this->getData(line, this->stack);
		}
		this->cnt = 0;
	}
	else {
		// keep data
		this->cnt %= lineLen;
		this->stack[cnt] = c;
		this->cnt++;
	}

	return this->enable;
}

bool READLINE::isConnected() {
	return this->SP->IsConnected();
}

#endif READLINE_H
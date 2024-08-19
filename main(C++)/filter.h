#ifndef FILTER_H
#define FILTER_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
using namespace std;

class FILTER {
private:
	int size = 1;
	int alpha = 0;
	int idx = 0;
	int avgSize = 0;
	vector<int> stack;

public:
	FILTER(int size, int alpha) : size(size), alpha(alpha) {
		this->stack = vector<int>(size, 0);
		this->avgSize = size - (alpha * 2);
	}

	int filtering(int v) {
		idx++;
		if (idx == size) idx = 0;
		vector<int> tmp;
		int sum = 0;

		this->stack[idx] = v;
		tmp = this->stack;
		sort(tmp.begin(), tmp.end());

		for (int i = (this->alpha); i < size - (this->alpha); i++) sum += tmp[i];
		return sum / (this->avgSize);
	}
};

#endif FILTER_H

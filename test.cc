#include <stdio.h>
#include <sstream>
#include <string>
#include <stdlib.h>

#include <iostream>

using namespace std;

string toHex(double x) {
	unsigned char * array = (unsigned char * ) & x;
	int len = sizeof(double) - 1;
	char * buf = new char[3];
	stringstream ss;
	for (int i = len; i >= 0; i--) {
		snprintf(buf, 3, "%02X", array[i]);
		ss << buf;
	}
	return ss.str();
}

string toHex(float x) {
	unsigned char * array = (unsigned char * ) & x;
	int len = sizeof(float) - 1;
	char * buf = new char[3];
	stringstream ss;
	for (int i = len; i >= 0; i--) {
		snprintf(buf, 3, "%02X", array[i]);
		ss << buf;
	}
	return ss.str();
}


int main() {
	float x = 1.23;
	string hex = toHex(x);
	double x1 = toFloat(hex);
	cout << x << endl;
	cout << hex << endl;
	cout << x1 << endl;
}

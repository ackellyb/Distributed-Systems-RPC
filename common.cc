/*
 * Common.cc
 *
 *  Created on: Jul 15, 2014
 *      Author: Aaron
 */

#include "common.h"

int getArrayLen(int type) {
	int findArrayLen = 65535; //16 0s followed by 16 1s
	int arrayLen = type & findArrayLen;
	return arrayLen;
}

int getpType(int t) {
	int findType = 255 << 16; //8 0s followed by 8 1s followed by 16 0s
	return  (t & findType) >> 16;
}

string toHex(unsigned char * array, int len) {
	char * buf = new char[3];
	stringstream ss;
	for (int i = len; i >= 0; i--) {
		snprintf(buf, 3, "%02X", array[i]);
		ss << buf;
	}
	delete [] buf;
	return ss.str();
}

string dToHex(double x) {
	unsigned char * array = (unsigned char * ) & x;
	int len = sizeof(double) - 1;
	return toHex(array, len);
}

string flToHex(float x) {
	unsigned char * array = (unsigned char * ) & x;
	int len = sizeof(float) - 1;
	return toHex(array, len);
}

double toDouble(string x) {
	union {
		long long i;
		double d;
	} value;

	value.i = strtoll(x.c_str(), 0, 16);

	return value.d;
}

float toFloat(string x) {
	union {
		long i;
		float d;
	} value;

	value.i = strtoll(x.c_str(), 0, 16);

	return value.d;
}

string getKey(string name, string argTypeStr) {
	stringstream ss;
	stringstream ssOut;
	ss.str(argTypeStr);
	ssOut << name << ",";
	string stemp;
	int withOutLen;
	int removeLen = 65535 << 16; //16 1s followed by 16 0s
	while (getline(ss, stemp, '#')) {
		int withLen = atoi(stemp.c_str());
		//eleminate lower 16 bits
		withOutLen = withLen & removeLen;
		if (withLen != withOutLen) {
			//mark as array
			withOutLen = withOutLen | 1;
		}
		ssOut << withLen << "#";
	}
	return ssOut.str();
}


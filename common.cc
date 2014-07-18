/*
 * Common.cc
 *
 *  Created on: Jul 15, 2014
 *      Author: Aaron
 */

#include "common.h"
#include "err.h"

int getArrayLen(int type) {
	int findArrayLen = 65535; //16 0s followed by 16 1s
	int arrayLen = type & findArrayLen;
	return arrayLen;
}

int getpType(int type) {
	int findType = 255 << 16; //8 0s followed by 8 1s followed by 16 0s
	return  (type & findType) >> 16;
}

bool isOnlyOutput(int type) {
	unsigned int bitMask = 3 << 30;
	unsigned int IOType = type & bitMask;
	IOType = IOType >> 30;
	return IOType == 1;
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
		ssOut << withOutLen << "#";
	}
	return ssOut.str();
}

int * vectorToArray(vector<int> vec) {
	int * array = new int[vec.size()];
	for (int i = 0; i < vec.size(); i++) {
		array[i] = vec[i];
	}
	return array;
}


string convertToString(char* c) {
	stringstream ss;
	string s;
	ss < c;
	ss >> s;
	return s;
}

int getConnection(int s) {
	int t;
	if ((t = accept(s, NULL, NULL )) < 0) { /* accept connection if there is one */
		return (-1);
	}
	return (t);
}


int createSocket(char* addr, char* prt) {
	char * address;
	char * port;
	if (addr == NULL ) {//if no address specified get fron environment
		address = getenv("BINDER_ADDRESS");
		port = getenv("BINDER_PORT");
	} else {
		address = addr;
		port = prt;
	}

	int retVal;

	if (address == NULL || port == NULL ) {
		return CANT_CREATE_SOCKET_ERR;
	}
	//Connecting to binder
	int soc = socket(AF_INET, SOCK_STREAM, 0);
	if (soc < 0) {
		return CANT_CREATE_SOCKET_ERR;
	}

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(address, port, &hints, &res);
	retVal = connect(soc, res->ai_addr, res->ai_addrlen);
	if (retVal < 0) {
		freeaddrinfo(res);
		return CANT_CONNECT_SOCKET_ERR;
	}
	freeaddrinfo(res);
	return soc;
}


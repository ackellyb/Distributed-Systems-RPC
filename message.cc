/*
 * message.cc
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */
#include "message.h"
#include "rpc.h"


Message::Message() {
	// TODO Auto-generated constructor stub
}

Message::Message(int type, char * message) {
	this->type = type;
	this->message = message;
	this->len = strlen(message)+1;
}

Message::Message(int type, string message) {
	this->len = message.length()+1;
	this->message = new char[this->len];
	strcpy(this->message, message.c_str());
	this->type = type;
}

Message::~Message() {
	// TODO Auto-generated destructor stub
}

void Message::sendMessage(int port) {
	convert.i = this->len;
	send(port, convert.bits, 32, 0);
	convert.i = this->type;
	send(port, convert.bits, 32, 0);
	send(port, this->message, this->len, 0);
}

int Message::receiveMessage(int port) {
	int retVal = recv(port, convert.bits, 32, 0);
	if (retVal <= 0) {
		return retVal;
	}
	this->len = convert.i;
	retVal = recv(port, convert.bits, 32, 0);
	if (retVal <= 0) {
		return retVal;
	}
	this->type = convert.i;
	this->message = new char[len];
	retVal = recv(port, this->message, this->len, 0);
	return retVal;
}

int Message::getType() {
	return this->type;
}

int Message::getLength() {
	return this->len;
}
char * Message::getMessage() {
	return this->message;
}


bool isArray(int type) {
	int findArrayLen = 65535; //16 0s followed by 16 1s
	int arrayLen = type & arrayLen;
	return arrayLen > 0;
}

int getpType(int t) {
	int findType = 255 << 16; //8 0s followed by 8 1s followed by 16 0s
	return t & findType >> 16;
}

string toHex(unsigned char * array, int len) {
	char * buf = new char[3];
	stringstream ss;
	for (int i = len; i >= 0; i--) {
		snprintf(buf, 3, "%02X", array[i]);
		ss << buf;
	}
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


//commas separate args, # separates array elements
//length, msgCode, hostname, port, name, argTypes
string createRegisterMsg(int port, char *name, int *argTypes) {
	char address[256];
	gethostname(address, 256);
	stringstream ss;
	ss << address << "," << port << "," << name << ",";
	int i=0;
	while(argTypes[i]!= 0) {
		ss << argTypes[i] << "#";
		i++;
	}
	ss << argTypes[i] << "#";
	return ss.str();
}

string createCodeMsg(int code) {
	stringstream ss;
	ss << code;
	return ss.str();
}

//not sure when this will be used

string createLocRequestMsg(char *name, int *argTypes) {
	stringstream ss;
	ss  << name << ",";
	int i=0;
	while(argTypes[i]!= 0) {
		ss << argTypes[i] << "#";
		i++;
	}
	ss << argTypes[i] << "#";
	return ss.str();
}

string createLocSuccessMsg(string host, int port) {
	stringstream ss;
	ss << host << "," << port;
	return ss.str();
}

//msgType will donote weather it is just an execute or an executeSuccess
string createExecuteMsg(char *name, int *argTypes, void** args) { //args
	stringstream ss;
	ss << name << ",";
	int lengthArray = sizeof(argTypes) / sizeof(*argTypes);
//	for (int i = 0; i <= lengthArray; i++) {
	int i=0;
		while(argTypes[i]!= 0) {
		ss << argTypes[i] << "#";
		i++;
	}
		ss << argTypes[i] << "#";
	ss << ", ";
	for (int i = 0; i <= lengthArray; i++) {
		int type = argTypes[i];
		int ptype = getpType(type);
		if (isArray(type)) {
			//cast it into array type...
			if (ptype == ARG_CHAR) { //char
				char * array = (char*)args[i];
				int innerArrayLength = sizeof(array) / sizeof(*array);
				for (int j = 0; j <= innerArrayLength; j++) {
					ss << array[j] << ";";
				}

			} else if (ptype == ARG_SHORT) { //short
				short * array = (short*)args[i];
				int innerArrayLength = sizeof(array) / sizeof(*array);
				for (int j = 0; j <= innerArrayLength; j++) {
					ss << array[j] << ";";
				}
			} else if (ptype == ARG_INT) { //int
				int * array = (int*)args[i];
				int innerArrayLength = sizeof(array) / sizeof(*array);
				for (int j = 0; j <= innerArrayLength; j++) {
					ss << array[j] << ";";
				}

			} else if (ptype == ARG_LONG) { //long
				long * array = (long*)args[i];
				int innerArrayLength = sizeof(array) / sizeof(*array);
				for (int j = 0; j <= innerArrayLength; j++) {
					ss << array[j] << ";";
				}

			} else if (ptype == ARG_DOUBLE) { //double
				double * array = (double*)args[i];
				int innerArrayLength = sizeof(array) / sizeof(*array);
				for (int j = 0; j <= innerArrayLength; j++) {
					ss << dToHex((double)array[j]) << ";";
				}

			} else if (ptype == ARG_FLOAT) { //float
				float * array = (float*)args[i];
				int innerArrayLength = sizeof(array) / sizeof(*array);
				for (int j = 0; j <= innerArrayLength; j++) {
					ss << flToHex((float)array[j]) << ";";
				}

			}
			ss << "#";
		} else {
			if (ptype == ARG_DOUBLE) {
				double * arg = (double *) args[i];
				ss << dToHex(*arg) << ";";
			} else if (ptype == ARG_FLOAT) {
				float * arg = (float *) args[i];
				ss << flToHex(*arg) << ";";
			} else {
				ss << args[i] << "#";
			}
		}
	}

	return ss.str();
}

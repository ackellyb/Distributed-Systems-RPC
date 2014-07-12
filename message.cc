/*
 * message.cc
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */
#include "message.h"
#include "rpc.h"
#include <sstream>
#include <cstring>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

bool isArray(int type) {
	int findArrayLen = 65535; //16 0s followed by 16 1s
	int arrayLen = type & arrayLen;
	return arrayLen > 0;
}

int getpType(int t) {
	int findType = 255 << 16; //8 0s followed by 8 1s followed by 16 0s
	return t & findType >> 16;
}

//commas separate args, # separates array elements
//length, msgCode, hostname, port, name, argTypes
string createRegisterMsg(int port, char *name, int *argTypes) {
	char address[256];
	gethostname(address, 256);
	stringstream ss;
	ss << MSG_REGISTER << "," << address << "," << port << "," << name << ",";
	int lengthArray = sizeof(argTypes) / sizeof(*argTypes);
	for (int i = 0; i <= lengthArray; i++) {
		ss << argTypes[i] << "#";
	}
	string msg;
	msg = ss.str();
	//not sure if we want length
//	  int len = strlen(msg.c_str())+1;
//	  ss.str("");
//	  ss << len <<"," << msg;
//	  msg = ss.str();
//	  cout<< msg << endl;
	return msg;
}

string createRegisterSuccessMsg() {
	stringstream ss;
	ss << MSG_REGISTER_SUCCEESS;
	return ss.str();
}

//not sure when this will be used
string createRegisterFailtureMsg(int reasonCode) {
	stringstream ss;
	ss << MSG_REGISTER_FAILURE << "," << reasonCode;
	return ss.str();
}

string createLocRequestMsg(char *name, int *argTypes) {
	stringstream ss;
	ss << MSG_LOC_REQUEST << "," << name << ",";
	int lengthArray = sizeof(argTypes) / sizeof(*argTypes);
	for (int i = 0; i <= lengthArray; i++) {
		ss << argTypes[i] << "#";
	}
	return ss.str();
}

string createLocSuccessMsg(string host, int port) {
	stringstream ss;
	ss << MSG_LOC_SUCCESS << "," << host << "," << port;
	return ss.str();
}

string createFailureMsg(int msgType, int reasonCode) {
	stringstream ss;
	ss << msgType << "," << reasonCode;
	return ss.str();
}

//msgType will donote weather it is just an execute or an executeSuccess
//this encoding wont work for decimals :(
string createExecuteMsg(int msgType, char *name, int *argTypes, void** args) { //args
	stringstream ss;
	ss << msgType << "," << name << ",";
	int lengthArray = sizeof(argTypes) / sizeof(*argTypes);
	for (int i = 0; i <= lengthArray; i++) {
		ss << argTypes[i] << "#";
	}
	ss << ", ";
	for (int i = 0; i <= lengthArray; i++) {
		int type = argTypes[i];
		int ptype = getpType(type);
		if (isArray(type)) {
			//cast it into array type...
			if (ptype == ARG_CHAR) { //char
				char * array = (char*)args[i];
				int innerArrayLength = sizeof(array) / sizeof(*array);
				for (int j = 0; i <= innerArrayLength; j++) {
					ss << array[j] << ";";
				}

			} else if (ptype == ARG_SHORT) { //short
				short * array = (short*)args[i];
				int innerArrayLength = sizeof(array) / sizeof(*array);
				for (int j = 0; i <= innerArrayLength; j++) {
					ss << array[j] << ";";
				}
			} else if (ptype == ARG_INT) { //int
				int * array = (int*)args[i];
				int innerArrayLength = sizeof(array) / sizeof(*array);
				for (int j = 0; i <= innerArrayLength; j++) {
					ss << array[j] << ";";
				}

			} else if (ptype == ARG_LONG) { //long
				long * array = (long*)args[i];
				int innerArrayLength = sizeof(array) / sizeof(*array);
				for (int j = 0; i <= innerArrayLength; j++) {
					ss << array[j] << ";";
				}

			} else if (ptype == ARG_DOUBLE) { //double
				double * array = (double*)args[i];
				int innerArrayLength = sizeof(array) / sizeof(*array);
				for (int j = 0; i <= innerArrayLength; j++) {
					ss << array[j] << ";";
				}

			} else if (ptype == ARG_FLOAT) { //float
				float * array = (float*)args[i];
				int innerArrayLength = sizeof(array) / sizeof(*array);
				for (int j = 0; i <= innerArrayLength; j++) {
					ss << array[j] << ";";
				}

			}
			ss << "#";
		} else {
			ss << args[i] << "#";
		}
	}

	string msg;
	msg = ss.str();
	//not using length for now
//	 	  int len = strlen(msg.c_str())+1;
//	 	  ss.str("");
//	 	  ss << len <<"," << msg;
//	 	  msg = ss.str();
//	 	  cout<< msg << endl;
	return msg;
}

string createTerminate() {
	return "10";
}

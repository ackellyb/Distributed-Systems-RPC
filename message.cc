/*
 * message.cc
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */
#include "message.h"
#include "rpc.h"
#include "common.h"


Message::Message() {
	this->message = NULL;
}

Message::Message(int type, char * message) {
	this->type = type;
	this->len = strlen(message)+1;
	this->message = new char[this->len];
	strcpy(this->message, message);
}

Message::Message(int type, string message) {
	this->len = message.length()+1;
	this->message = new char[this->len];
	strcpy(this->message, message.c_str());
	this->type = type;
}

Message::~Message() {
	if(message != NULL){
	delete []message;
	}
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

string Message::getTypeString() {
	switch(this->type){
		case MSG_REGISTER:
			return "Register";
		case MSG_REGISTER_SUCCEESS:
			return "Register Success";
		case MSG_REGISTER_FAILURE:
			return "Register failure";
		case MSG_LOC_REQUEST:
			return "Location request";
		case MSG_LOC_SUCCESS:
			return "Location success";
		case MSG_LOC_FAILURE:
			return "Location failure";
		case MSG_EXECUTE:
			return "Execute";
		case MSG_EXECUTE_SUCCESS:
			return "Execute success";
		case MSG_EXECUTE_FAILURE:
			return "Execute failure";
		case MSG_TERMINATE:
			return "Terminate";
		default:
			return "Message type is not known";
	}
}

int Message::getLength() {
	return this->len;
}
char * Message::getMessage() {
	return this->message;
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


string createExecuteMsg(char *name, int *argTypes, void** args) { //args
	stringstream ss;
	ss << name << ",";
	int lengthArray = 0;
	while(argTypes[lengthArray]!= 0) {
		ss << argTypes[lengthArray] << "#";
		lengthArray++;
	}
	ss << argTypes[lengthArray] << "#";
	ss <<  ",";
	for (int i = 0; i < lengthArray; i++) {
		int type = argTypes[i];
		int ptype = getpType(type);
		int arrayLen = getArrayLen(type);

		if (arrayLen > 0) {
			//cast it into array type...
			if (ptype == ARG_CHAR) { //char
				char * array = (char*)args[i];
				for (int j = 0; j < arrayLen; j++) {
					ss << array[j] << ";";
				}

			} else if (ptype == ARG_SHORT) { //short
				short * array = (short*)args[i];
				for (int j = 0; j < arrayLen; j++) {
					ss << array[j] << ";";
				}
			} else if (ptype == ARG_INT) { //int
				int * array = (int*)args[i];
				for (int j = 0; j < arrayLen; j++) {
					ss << array[j] << ";";
				}

			} else if (ptype == ARG_LONG) { //long
				long * array = (long*)args[i];
				for (int j = 0; j < arrayLen; j++) {
					ss << array[j] << ";";
				}

			} else if (ptype == ARG_DOUBLE) { //double
				double * array = (double*)args[i];
				for (int j = 0; j < arrayLen; j++) {
					ss << dToHex((double)array[j]) << ";";
				}

			} else if (ptype == ARG_FLOAT) { //float
				float * array = (float*)args[i];
				for (int j = 0; j < arrayLen; j++) {
					ss << flToHex((float)array[j]) << ";";
				}
			} else {
				ss << "NULL" << ";";
			}
			ss << "#";
		} else {
			if (ptype == ARG_DOUBLE) {
				double * arg = (double *) args[i];
				string msg = dToHex(*arg);
				ss << msg.c_str() << "#";
			} else if (ptype == ARG_FLOAT) {
				float * arg = (float *) args[i];
				string msg = flToHex(*arg);
				ss << msg << "#";
			} else if(ptype == ARG_LONG) {
				long * arg = (long*) args[i];
				ss << *arg << "#";
			} else if (ptype == ARG_INT) {
				int * arg = (int*)args[i];
				ss << *arg << "#";
			} else if (ptype == ARG_SHORT) {
				short * arg = (short*)args[i];
				ss << *arg << "#";
			} else if (ptype == ARG_CHAR) {
				char * arg = (char*)args[i];
				ss << *arg << "#";
			} else {
				ss << "NULL" << "#";
			}
		}
	}

	return ss.str();
}

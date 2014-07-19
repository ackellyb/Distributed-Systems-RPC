/*
 * message.cc
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */
#include "message.h"
#include "rpc.h"
#include "common.h"
#include "err.h"


Message::Message() {
	this->message = NULL;
}


//Constructor with c style character
Message::Message(int type, char * message) {
	this->type = type;
	this->len = strlen(message) + 1;
	this->message = new char[this->len];
	strcpy(this->message, message);
}


//Constructor with c++ string
Message::Message(int type, string message) {
	this->len = message.length() + 1;
	this->message = new char[this->len];
	strcpy(this->message, message.c_str());
	this->type = type;
}

Message::Message(int type) {
	this->len = 1;
	this->message = '\0';
	this->type = type;
}


Message::~Message() {
	if (message != NULL ) {
		delete[] message;
	}
}


// Sends the message to the port specified in the input
// Sends length first, then type, followed by the message contents
// Returns 0 if successful
// Returns MESSAGE_SEND_ERR if there is an error with sending any of the objects
int Message::sendMessage(int port) {
	int32_t lenT = htonl(this->len);
	int32_t typeT = htonl(this->type);

	int retVal = send(port, &lenT, sizeof(int32_t), 0);
	if (retVal <= 0) {
		return MESSAGE_SEND_ERR;
	}

	retVal = send(port, &typeT, sizeof(int32_t), 0);
	if (retVal <= 0) {
		return MESSAGE_SEND_ERR;
	}

	if (this->len > 1) {
		retVal = send(port, this->message, this->len, 0);
		if (retVal <= 0) {
			return MESSAGE_SEND_ERR;
		}
	}

	return SUCCESS;
}

// Receives the message from the specified port, and populates the message object
// Receives length first, then type, then the message
// Returns 0 if successful
// Returns MESSAGE_RECEIVE_ERR if any object was not able to be received
int Message::receiveMessage(int port) {
	int32_t lenT = 0;
	int32_t typeT = 0;

	int retVal = recv(port, &lenT, sizeof(int32_t), 0);
	if (retVal <= 0) {
		return MESSAGE_RECEIVE_ERR;
	}
	this->len = ntohl(lenT);

	retVal = recv(port, &typeT, sizeof(int32_t), 0);
	if (retVal <= 0) {
		return MESSAGE_RECEIVE_ERR;
	}
	this->type = ntohl(typeT);


	if (this->len > 1) {
		this->message = new char[len];
		retVal = recv(port, this->message, this->len, 0);
		if (retVal <= 0) {
			return MESSAGE_RECEIVE_ERR;
		}
	} else {
		this->message = '\0';
	}


	return SUCCESS;
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



// Creates the message string for Register messages [address, port, name, argTypes]
// Returns the created string
// returnVal is set to HOST_NOT_FOUND if the host address is not found
string createRegisterMsg(int port, char *name, int *argTypes, int * returnVal) {
	char address[256];
	*returnVal = 0;
	int retVal = gethostname(address, 256);
	if (retVal != 0) {
		*returnVal =  HOST_NOT_FOUND_ERR;
	}
	stringstream ss;
	ss << address << "," << port << "," << name << ",";
	int i = 0;
	while (argTypes[i] != 0) {
		ss << argTypes[i] << "#";
		i++;
	}
	ss << argTypes[i] << "#";
	return ss.str();
}


// Creates a simple message string, where the only thing in the message is a code [errNo]
// Returns the code message
string createCodeMsg(int code) {
	stringstream ss;
	ss << code;
	return ss.str();
}


// Creates a location request message [name, argTypes]
// Returns the string
string createLocRequestMsg(char *name, int *argTypes) {
	stringstream ss;
	ss << name << ",";
	int i = 0;
	while (argTypes[i] != 0) {
		ss << argTypes[i] << "#";
		i++;
	}
	ss << argTypes[i] << "#";
	return ss.str();
}


// Creates a location request success message [host, port]
// Returns the message
string createLocSuccessMsg(string host, int port) {
	stringstream ss;
	ss << host << "," << port;
	return ss.str();
}


// Creates an execute message request and execute success message string [name, argTypes, args]
// If the message is a success message, isSuccessMessage is true. Any output variables are correctly parsed
// If the message is not a success message, output variables are set in the message to say "output". This stops reads from unallocated memory
// Returns a string
string createExecuteMsg(char *name, int *argTypes, void** args, bool isSuccessMessage) {
	stringstream ss;
	ss << name << ",";

	//Calculate length, and parse argTypes into stringstream
	int lengthArray = 0;
	while (argTypes[lengthArray] != 0) {
		ss << argTypes[lengthArray] << "#";
		lengthArray++;
	}
	ss << argTypes[lengthArray] << "#";
	ss << ",";

	//Parse args into stringstream
	for (int i = 0; i < lengthArray; i++) {
		int type = argTypes[i];
		int ptype = getpType(type);
		bool isOutput = isOnlyOutput(type) && !isSuccessMessage;
		int arrayLen = getArrayLen(type);

		if (isOutput) {
			ss << "output#";
		} else if (arrayLen > 0) {
			//cast it into array type...
			if (ptype == ARG_CHAR) { //char
				char * array = (char*) args[i];
				for (int j = 0; j < arrayLen; j++) {
					ss << (int)array[j] << ";";
				}
			} else if (ptype == ARG_SHORT) { //short
				short * array = (short*) args[i];
				for (int j = 0; j < arrayLen; j++) {
					ss << array[j] << ";";
				}
			} else if (ptype == ARG_INT) { //int
				int * array = (int*) args[i];
				for (int j = 0; j < arrayLen; j++) {
					ss << array[j] << ";";
				}

			} else if (ptype == ARG_LONG) { //long
				long * array = (long*) args[i];
				for (int j = 0; j < arrayLen; j++) {
					ss << array[j] << ";";
				}

			} else if (ptype == ARG_DOUBLE) { //double
				double * array = (double*) args[i];
				for (int j = 0; j < arrayLen; j++) {
					ss << dToHex((double) array[j]) << ";";
				}

			} else if (ptype == ARG_FLOAT) { //float
				float * array = (float*) args[i];
				for (int j = 0; j < arrayLen; j++) {
					ss << flToHex((float) array[j]) << ";";
				}
			} else {
				ss << "NULL";
			}
			ss << "#";
		} else { //scalar variable
			if (ptype == ARG_DOUBLE) {
				double * arg = (double *) args[i];
				string msg = dToHex(*arg);
				ss << msg.c_str() << "#";
			} else if (ptype == ARG_FLOAT) {
				float * arg = (float *) args[i];
				string msg = flToHex(*arg);
				ss << msg << "#";
			} else if (ptype == ARG_LONG) {
				long * arg = (long*) args[i];
				ss << *arg << "#";
			} else if (ptype == ARG_INT) {
				int * arg = (int*) args[i];
				ss << *arg << "#";
			} else if (ptype == ARG_SHORT) {
				short * arg = (short*) args[i];
				ss << *arg << "#";
			} else if (ptype == ARG_CHAR) {
				char * arg = (char*) args[i];
				ss << (int)(*arg) << "#";
			} else {
				ss << "NULL" << "#";
			}
		}
	}

	return ss.str();
}

string createExecuteSuccessMsg(char * name, int * argTypes, void ** args) {
	return createExecuteMsg(name, argTypes, args, true);
}

string createExecuteRequestMsg(char * name, int * argTypes, void ** args) {
	return createExecuteMsg(name, argTypes, args, false);
}

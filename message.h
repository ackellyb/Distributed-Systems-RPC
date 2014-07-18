/*
 * message.h
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <cstring>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#ifndef MESSAGE_H_
#define MESSAGE_H_

#define MSG_REGISTER      			1
#define MSG_REGISTER_SUCCEESS      	2
#define MSG_REGISTER_FAILURE      	3
#define MSG_LOC_REQUEST   			4
#define MSG_LOC_SUCCESS   			5
#define MSG_LOC_FAILURE   			6
#define MSG_EXECUTE 		  		7
#define MSG_EXECUTE_SUCCESS    		8
#define MSG_EXECUTE_FAILURE   		9
#define MSG_TERMINATE     			10

using namespace std;

class Message {
	private:
		int type;
		int len;
		char * message;
	public:
		Message();
		Message(int type, char * message);
		Message(int type, string message);
		virtual ~Message();

		void sendMessage(int port);
		int receiveMessage(int port);
		int getType();
		string getTypeString();
		char * getMessage();
		int getLength();
};

string createRegisterMsg(int port, char *name, int *argTypes);
string createLocRequestMsg(char *name, int *argTypes);
string createLocSuccessMsg(string host, int port);
string createExecuteMsg(char *name, int *argTypes, void** args, bool isSuccessMessage);
string createCodeMsg(int code);


#endif /* MESSAGE_H_ */

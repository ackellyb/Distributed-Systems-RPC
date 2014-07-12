/*
 * message.h
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */

#include <string>
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


string createRegisterMsg(int port, char *name, int *argTypes);
string createRegisterSuccessMsg();
string createLocRequestMsg(char *name, int *argTypes);
string createLocSuccessMsg(string host, int port);
string createFailureMsg(int msgType, int reasonCode);
string createExecuteMsg(int msgType, char *name, int *argTypes, void** args);
string createTerminate();

#endif /* MESSAGE_H_ */

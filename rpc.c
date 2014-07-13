/*
 * rpc.c
 *
 *  Created on: Jul 3, 2014
 *      Author: Aaron
 */

#include "rpc.h"
#include "message.h"
#include "err.h"
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <cstring>
#include <map>
#include <iostream>
#include <pthread.h>
#define MAX_CONNECTIONS 100
#define HOST_NAME_SIZE 256
int MAX_SIZE = 2 ^ 32 - 1;

int binderSocket, clientSocket, serverPort;
char address[HOST_NAME_SIZE];
//lock this shit up
map <string, skeleton> localDb;
volatile int runningThreads = 0;
pthread_mutex_t runningLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t localDbLock = PTHREAD_MUTEX_INITIALIZER;

string convertToString(char* c) {
	stringstream ss;
	string s;
	ss < c;
	ss >> s;
	return s;
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
		//elemenate lower 16 bits
		withOutLen = withLen & removeLen;
		if (withLen != withOutLen) {
			//mark as array
			withOutLen = withOutLen | 1;
		}
		ssOut << withLen << "#";
	}
	return ssOut.str();
}

int createSocket( char* addr, char* prt) {
	char * address;
	char * port;
	if (addr == NULL ) {
		address = getenv("BINDER_ADDRESS");
		port = getenv("BINDER_PORT");
	} else {
		address = addr;
		port = prt;
	}

	int retVal;

	if (address == NULL || port == NULL ) {
		return -1;
	}
	//Connecting to binder
	int cBinderSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (cBinderSocket == -1) {
		return -1;
	}

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(address, port, &hints, &res);
	retVal = connect(cBinderSocket, res->ai_addr, res->ai_addrlen);
	if (retVal == -1) {
		return -1;
	}
	return cBinderSocket;
}

//Server Function
int rpcInit() {
	char * address = getenv("BINDER_ADDRESS");
	char * port = getenv("BINDER_PORT");
	int retVal;

	if (address == NULL || port == NULL ) {
		return -1;
	}

	cout << "init called" << endl;
	//Connecting to binder
	binderSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (binderSocket == -1) {
		return -1;
	}

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(address, port, &hints, &res);
	retVal = connect(binderSocket, res->ai_addr, res->ai_addrlen);
	if (retVal == -1) {
		return -1;
	}

	//Opening connection for clients
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == -1) {
		return -1;
	}

	struct sockaddr_in sockAddress;
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = INADDR_ANY;
	sockAddress.sin_port = htons(0);
	retVal = bind(clientSocket, (struct sockaddr *) &sockAddress,
			sizeof(sockAddress));
	if (retVal == -1) {
		return -1;
	}
	retVal = listen(clientSocket, MAX_CONNECTIONS);
	if (retVal == -1) {
		return -1;
	}

	struct sockaddr_in sin;
	int addrlen = sizeof(sin);
	getsockname(clientSocket, (struct sockaddr *) &sin, (socklen_t*) &addrlen);
	serverPort = ntohs(sin.sin_port);
	if (serverPort <= 0) {
		return -1;
	}
//	gethostname(address, HOST_NAME_SIZE);
//	if (address == NULL) {
//		return -1;
//	}
	cout << "init done" << endl;
	return SUCCESS;
}

int rpcRegister(char *name, int *argTypes, skeleton f) {
	//insert into local db
	stringstream ss;
	ss << name;
	cout << "registering" << endl;
	//check if it has already been registered if so dont bind...
//	if(){//only send if it is a new signature
//optional but would be cleaner to do...
//	}
	string msg = createRegisterMsg(serverPort, name, argTypes);
	//send register msg
	int len = strlen(msg.c_str()) + 1;
	send(binderSocket, msg.c_str(), len, 0);
	cout << "sent: " << msg << endl;
	//listen for acknlowedgment
	char reply[10];
	recv(binderSocket, reply, len, 0);
	cout << "got reply" << reply << endl;
	string returnCode = strtok(reply, ",");
//	ss << reply;
//	int returnCode;
//	ss >> returnCode;
	if (atoi(returnCode.c_str()) == MSG_REGISTER_SUCCEESS) {
		//success insert into local db
		string argTypeStr;
		ss.str("");
		ss.str(msg);
		cout << "still alive " << msg << endl;
//		getline(ss, argTypeStr, ',');//length
		getline(ss, argTypeStr, ','); //type
		getline(ss, argTypeStr, ','); //add
		getline(ss, argTypeStr, ','); //port
		getline(ss, argTypeStr, ','); //name
		getline(ss, argTypeStr, ','); //argType
		string key = getKey(name, argTypeStr);
		cout << "still alive " << msg << endl;
		localDb[key] = f;
		return SUCCESS;
	} else {
		//error?
		string errorCode;
		errorCode = strtok(NULL, ",");
		return atoi(errorCode.c_str());
	}
}

int rpcCall(char* name, int* argTypes, void** args) {
	//connect to binder
	cout << "rpcCall" << endl;
	int clientBinderSocket = createSocket(NULL, NULL );

//loc_request to binder
	string msg = createLocRequestMsg(name, argTypes);
	int len = strlen(msg.c_str()) + 1;
	send(clientBinderSocket, msg.c_str(), len, 0);

	// recieve msg
	char incommingMsg[MAX_SIZE];
	if (recv(clientBinderSocket, incommingMsg, MAX_SIZE, 0) <= 0) {
		close(clientBinderSocket);
		return -1;
	}
	close(clientBinderSocket);
	string msgType = strtok(incommingMsg, ",");

	if (atoi(msgType.c_str()) == MSG_LOC_SUCCESS) {
		cout << "msg_LOC_suc" << endl;
		char* serverHostName = strtok(NULL, ",");
		char* serverPortStr = strtok(NULL, ",");
//		int serverPort = atoi(serverPortStr.c_str());
		cout << "got back " << serverHostName << " " << serverPortStr << endl;

		//create connection to server
		int serverSocket = createSocket(serverHostName,  serverPortStr);
		if(serverSocket>0){
			cout<<"serverup"<<endl;
		}else{//server is not up
			cout<<"server down"<<endl;
			return -1;
		}
		//send execute message

		//wait for response from server

	} else if (atoi(msgType.c_str()) == MSG_LOC_FAILURE) { //errored
		cout << "msg_LOC_fail" << endl;
		string errorCode = strtok(NULL, ",");
		return atoi(errorCode.c_str());
	} else {
		//huh?
		cout << "msg_ huh" << endl;
		return -1;
	}
//MADE IT
	return SUCCESS;
}

void *executeThread(void*){
//execute stuff
	pthread_mutex_lock(&runningLock);
	runningThreads--;
	pthread_mutex_unlock(&runningLock);
}

int rpcExecute() {
	cout << "rpcExecute" << endl;

	if (localDb.empty()) {
		return -1;
	}

	while (true) {
		//accept conections

		char len[20];
		recv(clientSocket, len, 20, 0);
		cout << len << endl;
		int msgLen = atoi(len);
		char msg[msgLen];
		recv(clientSocket, msg, msgLen, 0);
		cout << msg << endl;
		cout << "execute recieved" << endl;

		string msgType = strtok(msg, ",");
		if (atoi(msgType.c_str()) == MSG_EXECUTE) {
			pthread_mutex_lock(&runningLock);
			runningThreads++;
			pthread_mutex_unlock(&runningLock);
			//spawn threads pass msg, and socket
		} else if (atoi(msgType.c_str()) == MSG_TERMINATE) {
			break;
		}
	}
	//wait for threads to finish
	while (runningThreads > 0) {
		sleep(1);
	}
	close(clientSocket);
	return 0;

}

int rpcTerminate() {
//listen for execute msgs or terminate msgs may need extra threads
//execute stuff, send back success/ failure
	cout << "rpcTerminate" << endl;
	int clientBinderSocket = createSocket(NULL, NULL );
	//send term msg
	string termMsg = createTerminate();
	int len = strlen(termMsg.c_str()) + 1;
	send(clientBinderSocket, termMsg.c_str(), len, 0);
}



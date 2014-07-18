/*
 * rpc.c
 *
 *  Created on: Jul 3, 2014
 *      Author: Aaron
 */

#include "rpc.h"
#include "message.h"
#include "err.h"
#include "common.h"
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
#include <vector>
#include <iostream>
#include <pthread.h>

#define MAX_CONNECTIONS 100
#define HOST_NAME_SIZE 256
int MAX_SIZE = 2 ^ 32 - 1;

int binderSocket = -1;
int clientListenerSocket = -1;
int serverPort = -1;
char address[HOST_NAME_SIZE];
//lock this shit up
map <string, skeleton> localDb;
volatile int runningThreads = 0;
pthread_mutex_t runningLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t localDbLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t printLock = PTHREAD_MUTEX_INITIALIZER;

string convertToString(char* c) {
	stringstream ss;
	string s;
	ss << c;
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

	cout << "init called" << endl;
	//Connecting to binder
	binderSocket = createSocket(address, port);
	if (binderSocket <= 0) {
		return -1;
	}

	//Opening connection for clients
	clientListenerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientListenerSocket == -1) {
		return -1;
	}

	struct sockaddr_in sockAddress;
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = INADDR_ANY;
	sockAddress.sin_port = htons(0);
	retVal = bind(clientListenerSocket, (struct sockaddr *) &sockAddress,
			sizeof(sockAddress));
	if (retVal == -1) {
		return -1;
	}
	retVal = listen(clientListenerSocket, MAX_CONNECTIONS);
	if (retVal == -1) {
		return -1;
	}

	struct sockaddr_in sin;
	int addrlen = sizeof(sin);
	getsockname(clientListenerSocket, (struct sockaddr *) &sin,
			(socklen_t*) &addrlen);
	serverPort = ntohs(sin.sin_port);
	if (serverPort <= 0) {
		return -1;
	}

	cout << "init done" << endl;
	return SUCCESS;
}

int rpcRegister(char *name, int *argTypes, skeleton f) {
	//insert into local db
	if (binderSocket < 0) {
		return -1;
	}
	stringstream ss;
	ss << name;
	cout << "registering" << endl;
	//check if it has already been registered if so dont bind...
//	if(){//only send if it is a new signature
//optional but would be cleaner to do...
//	}

	Message
	sendMsg(MSG_REGISTER, createRegisterMsg(serverPort, name, argTypes));
	sendMsg.sendMessage(binderSocket);

	cout << "sent: " << sendMsg.getMessage() << endl;
	//listen for acknlowedgment
	Message recvMsg;
	recvMsg.receiveMessage(binderSocket);
	cout << "got reply" << recvMsg.getMessage() << endl;

	if (recvMsg.getType() == MSG_REGISTER_SUCCEESS) {

		string argTypeStr;
		ss.str("");
		ss.str(sendMsg.getMessage());
		cout << "still alive " << sendMsg.getMessage() << endl;
		getline(ss, argTypeStr, ','); //add
		getline(ss, argTypeStr, ','); //port
		getline(ss, argTypeStr, ','); //name
		getline(ss, argTypeStr, ','); //argType
		string key = getKey(name, argTypeStr);
		cout << "still alive " << sendMsg.getMessage() << endl;
		pthread_mutex_lock(&localDbLock);
		localDb[key] = f;
		pthread_mutex_unlock(&localDbLock);
		return SUCCESS;
	} else {
		//error?
		return atoi(recvMsg.getMessage());
	}
}

void printDEBUG(string s) {
	printDEBUG(s, printLock);
}

vector<int> parseArgTypes(string argTypeStr) {
	vector<int> argTypes;
	stringstream ss(argTypeStr);
	string argTypeTemp;
	while (ss.good()) {
		getline(ss, argTypeTemp, '#');
		argTypes.push_back(atoi(argTypeTemp.c_str()));
	}
	return argTypes;
}

void ** parseArguments(int * argTypes, int len, string argStr) {
	stringstream ss( argStr);
	stringstream debug;
	string argTemp;

	void ** args = (void **) malloc(len * sizeof(void *));
	for (int i = 0; i < len; i++) {
		int type = getpType(argTypes[i]);
		int arrayLen = getArrayLen(argTypes[i]);

		if (arrayLen > 0) { //is an array
			if (type == ARG_CHAR) {
				char * array = new
				char[arrayLen];
				for (int j = 0; j < arrayLen; j++) {
					getline(ss, argTemp, ';');
					array[j] = argTemp[0];
				}
				args[i] = (void*) array;
			} else if (type == ARG_SHORT) {
				short * array = new
				short[arrayLen];
				for (int j = 0; j < arrayLen; j++) {
					getline(ss, argTemp, ';');
					array[j] = (short) atoi(argTemp.c_str());
				}
				args[i] = (void*) array;
			} else if (type == ARG_INT) {
				int * array = new
				int[arrayLen];
				for (int j = 0; j < arrayLen; j++) {
					getline(ss, argTemp, ';');
					array[j] = atoi(argTemp.c_str());
				}
				args[i] = (void*) array;
			} else if (type == ARG_LONG) {
				long * array = new
				long[arrayLen];
				for (int j = 0; j < arrayLen; j++) {
					getline(ss, argTemp, ';');
					array[j] = atol(argTemp.c_str());
				}
				args[i] = (void*) array;
			} else if (type == ARG_FLOAT) {
				float * array = new
				float[arrayLen];
				for (int j = 0; j < arrayLen; j++) {
					getline(ss, argTemp, ';');
					debug.str("");
					array[j] = toFloat(argTemp);
					debug << array[j] << "-";
					printDEBUG(debug.str());
				}
				args[i] = (void*) array;
			} else if (type == ARG_DOUBLE) {
				double * array = new
				double[arrayLen];
				for (int j = 0; j < arrayLen; j++) {
					getline(ss, argTemp, ';');
					debug.str("");
					array[j] = toDouble(argTemp);
					debug << array[j] << "-";
					printDEBUG(debug.str());
				}
				args[i] = (void*) array;
			}
			getline(ss, argTemp, '#');
			printDEBUG("");
		} else { //not an array
			//need to free this later....
			getline(ss, argTemp, '#');
			debug.str("");
			if (type == ARG_CHAR) {
				char* arg = new
				char (argTemp[0]);
				args[i] = (void*) arg;
			} else if (type == ARG_SHORT) {
				short* arg = new
				short((short) atoi(argTemp.c_str()));
				args[i] = (void*) arg;
			} else if (type == ARG_INT) {
				int *arg = new
				int(atoi(argTemp.c_str()));
				args[i] = (void*) arg;
			} else if (type == ARG_LONG) {
				long* arg = new
				long(atol(argTemp.c_str()));
				args[i] = (void*) arg;
			} else if (type == ARG_FLOAT) {
				float* arg = new
				float (toFloat( argTemp));
				debug << *arg;
				printDEBUG(debug.str());
				args[i] = (void*) arg;
			} else if (type == ARG_DOUBLE) {
				double* arg = new
				double (toDouble( argTemp));
				debug << *arg;
				printDEBUG(debug.str());
				args[i] = (void*) arg;
			}

		}
	}
	return args;
}
void deleteArgValues(int * argTypes, int len, void** args) {
	for (int i = 0; i < len; i++) {
		int type = getpType(argTypes[i]);
		int arrayLen = getArrayLen(argTypes[i]);
		if (arrayLen > 0) { //is an array
			if (type == ARG_CHAR) {
				char * array = (char*) args[i];
				delete[] array;

			} else if (type == ARG_SHORT) {
				short * array = (short*) args[i];
				delete[] array;
			} else if (type == ARG_INT) {
				int * array = (int*) args[i];
				delete[] array;
			} else if (type == ARG_LONG) {
				long * array = (long*) args[i];
								delete[] array;
			} else if (type == ARG_FLOAT) {
				float * array = (float*) args[i];
								delete[] array;
			} else if (type == ARG_DOUBLE) {
				double * array = (double*) args[i];
								delete[] array;
			}
		} else {
			if (type == ARG_CHAR) {
				char* arg = (char*) args[i];
				delete arg;
			} else if (type == ARG_SHORT) {
				short* arg = (short*) args[i];
				delete arg;
			} else if (type == ARG_INT) {
				int* arg = (int*) args[i];
				delete arg;
			} else if (type == ARG_LONG) {
				long* arg = (long*) args[i];
				delete arg;
			} else if (type == ARG_FLOAT) {
				float* arg = (float*) args[i];
				delete arg;
			} else if (type == ARG_DOUBLE) {
				double* arg = (double*) args[i];
				delete arg;
			}
		}
	}

}
void copyArgValues(int * argTypes, int len, void** orgArgs, void** newArgs) {
	stringstream debug;
	for (int i = 0; i < len; i++) {
		int type = getpType(argTypes[i]);
		int arrayLen = getArrayLen(argTypes[i]);
		if (arrayLen > 0) { //is an array
			if (type == ARG_CHAR) {
				char * array = (char*) orgArgs[i];
				char * newArray = (char*) newArgs[i];
				for (int j = 0; j < arrayLen; j++) {
					array[j] = newArray[j];
				}
			} else if (type == ARG_SHORT) {
				short * array = (short*) orgArgs[i];
				short * newArray = (short*) newArgs[i];
				for (int j = 0; j < arrayLen; j++) {
					array[j] = newArray[j];
				}
			} else if (type == ARG_INT) {
				int * array = (int *) orgArgs[i];
				int * newArray = (int *) newArgs[i];
				for (int j = 0; j < arrayLen; j++) {
					array[j] = newArray[j];
				}
			} else if (type == ARG_LONG) {
				long * array = (long *) orgArgs[i];
				long * newArray = (long *) newArgs[i];
				for (int j = 0; j < arrayLen; j++) {
					array[j] = newArray[j];
				}
			} else if (type == ARG_FLOAT) {
				float * array = (float *) orgArgs[i];
				float * newArray = (float *) newArgs[i];
				for (int j = 0; j < arrayLen; j++) {
					array[j] = newArray[j];
				}
			} else if (type == ARG_DOUBLE) {
				double * array = (double *) orgArgs[i];
				double * newArray = (double *) newArgs[i];
				for (int j = 0; j < arrayLen; j++) {
					array[j] = newArray[j];
				}
			}
		} else {
			if (type == ARG_CHAR) {
				*((char*) orgArgs[i]) = *((char*) newArgs[i]);
			} else if (type == ARG_SHORT) {
				*((short*) orgArgs[i]) = *((short*) newArgs[i]);
			} else if (type == ARG_INT) {
				*((int*) orgArgs[i]) = *((int*) newArgs[i]);
			} else if (type == ARG_LONG) {
				*((long*) orgArgs[i]) = *((long*) newArgs[i]);
			} else if (type == ARG_FLOAT) {
				*((float*) orgArgs[i]) = *((float*) newArgs[i]);
//					debug << *arg;
//					printDEBUG(debug.str());
			} else if (type == ARG_DOUBLE) {
				*((double*) orgArgs[i]) = *((double*) newArgs[i]);
//					debug << *arg;
//					printDEBUG(debug.str());
			}
		}
	}
}

int rpcCall(char* name, int* argTypes, void** args) {
	//connect to binder
	cout << "rpcCall" << endl;
	int clientBinderSocket = createSocket(NULL, NULL );
	if (clientBinderSocket < 0) {
		return -1;
	}
//loc_request to binder
	Message
	locMsg(MSG_LOC_REQUEST, createLocRequestMsg(name, argTypes));
	locMsg.sendMessage(clientBinderSocket);

	// recieve msg
	Message locRecvMsg;
	if (locRecvMsg.receiveMessage(clientBinderSocket) <= 0) {
		close(clientBinderSocket);
		return -1;
	}
	close(clientBinderSocket);

	if (locRecvMsg.getType() == MSG_LOC_SUCCESS) {
		cout << "msg_LOC_suc" << endl;
//		locRecvMsg.getMessage()
		char* serverHostName = strtok(locRecvMsg.getMessage(), ",");
		char* serverPortStr = strtok(NULL, ",");
		cout << "got back " << serverHostName << " " << serverPortStr << endl;

		//create connection to server
		int serverSocket = createSocket(serverHostName, serverPortStr);
		if (serverSocket > 0) {
			cout << "serverup" << endl;
		} else { //server is not up
			cout << "server down" << endl;
			close(serverSocket);
			return -1;
		}

		Message
		serverExecuteMsg(MSG_EXECUTE, createExecuteMsg(name, argTypes, args));
		serverExecuteMsg.sendMessage(serverSocket);

		cout << "sent message: "<<serverExecuteMsg.getMessage() << endl;
		Message serverReceivedMsg;
		serverReceivedMsg.receiveMessage(serverSocket);
		close(serverSocket);
		if (serverReceivedMsg.getType() == MSG_EXECUTE_SUCCESS) {
			stringstream
			ss(serverReceivedMsg.getMessage());
			string name, argTypeStr, argStr;
			getline(ss, name, ',');
			getline(ss, argTypeStr, ',');
			getline(ss, argStr);

			vector<int> argTypesVec = parseArgTypes(argTypeStr);
//			argTypes = vectorToArray(argTypesVec);
			int length = argTypesVec.size() - 1;

			printDEBUG("what comes next are the variables");

			//dont think this is right, have to set values to be equal...
			void ** newArgs = parseArguments(argTypes, length, argStr);
			int* out;
			out = (int*) newArgs[0];
			cout << *out << endl;
			//delete newArgs....
			copyArgValues(argTypes, length, args, newArgs);
			deleteArgValues(argTypes, length, newArgs);
			cout << "msg_EXECUTE_SUCCESS" << endl;

			return 0;
		} else {
			cout << "msg_EXECUTE_FAILURE" << endl;
//			char * errorCode = serverReceivedMsg.getMessage();
			return -1;
		}

	} else if (locRecvMsg.getType() == MSG_LOC_FAILURE) { //errored
		cout << "msg_LOC_fail" << endl;
		string errorCode = locRecvMsg.getMessage();
		return atoi(errorCode.c_str());
	} else {
		//huh?
		cout << "msg_ huh" << endl;
		return -1;
	}
//MADE IT
	return SUCCESS;
}

class
ThreadArgs
{
public: int cSocket;
	Message* msg;
};

void *executeThread(void* tArg) {
//	string msg;
	ThreadArgs * tArgs = (ThreadArgs *) tArg;
	int cSocket = tArgs->cSocket;
	Message* msg = tArgs->msg;
	pthread_mutex_lock(&printLock);
	cout << "Message: " << msg->getMessage() << endl;
	pthread_mutex_unlock(&printLock);

	//parse stuff here
	stringstream ss;
	ss << msg->getMessage();
	string name, argTypeStr, argStr;
	getline(ss, name, ',');
	getline(ss, argTypeStr, ',');
	getline(ss, argStr);

	printDEBUG(name);

	vector<int> argTypesVec = parseArgTypes(argTypeStr);
	int * argTypes = vectorToArray(argTypesVec);
	int length = argTypesVec.size() - 1;

	printDEBUG("what comes next are the variables");

	void ** argArray = parseArguments(argTypes, length, argStr);

	string key = getKey(name, argTypeStr);
	pthread_mutex_lock(&localDbLock);
	skeleton f = localDb[key];
	pthread_mutex_unlock(&localDbLock);
	if (f != NULL ) {
		printDEBUG("got somthing");
		int * out = (int*) argArray[0];
		cout << "before" << (*out) << endl;
		int retVal = f(argTypes, argArray);
		out = (int*) argArray[0];
		cout << "after" << (*out) << endl;
		string skelMsg;
		int type;
		if (retVal == 0) {
			char * cstr = new
			char[name.length()+1];
			strcpy(cstr, name.c_str());
			skelMsg = createExecuteMsg(cstr, argTypes, argArray);
			printDEBUG(skelMsg);
			type = MSG_EXECUTE_SUCCESS;
		delete[] cstr;
	} else {
		skelMsg = createCodeMsg(retVal);
		type = MSG_EXECUTE_FAILURE;
	}
	Message executeSkel( type, skelMsg);
	executeSkel.sendMessage(cSocket);
	deleteArgValues(argTypes, length, argArray);
} else {
	printDEBUG("shit");
	string skelMsg = createCodeMsg(-1);
	Message
	executeFail(MSG_EXECUTE_FAILURE, skelMsg);
	executeFail.sendMessage(cSocket);
}

//run skeleton and reply to client using cSocket
delete msg;
delete tArgs;
close(cSocket);
//	pthread_mutex_lock(&runningLock);
//	runningThreads--;
//	pthread_mutex_unlock(&runningLock);

}

int rpcExecute() {
cout << "rpcExecute" << endl;
//needs to listen to bindersocket for terminate....use select between listner and binder sock?

if (localDb.empty()) {
	return -1;
}
fd_set master;
fd_set read_fds;
FD_ZERO(&master);
FD_ZERO(&read_fds);
FD_SET(clientListenerSocket, &master);
FD_SET(binderSocket, &master);
int fdmax;
if (clientListenerSocket > binderSocket) {
	fdmax = clientListenerSocket;
} else {
	fdmax = binderSocket;
}
vector < pthread_t > threads;
bool terminateFlag = false;
while (true) {
	read_fds = master;
	select(fdmax + 1, &read_fds, NULL, NULL, NULL );
	for (int i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &read_fds)) {
			if (i == clientListenerSocket) {
				//accept conections
				int clientSocket = getConnection(clientListenerSocket);
				if (clientSocket > 0) {
					Message *executeMsg = new
					Message();
					executeMsg->receiveMessage(clientSocket);

					if (executeMsg->getType() == MSG_EXECUTE) {
						cout << "execute recieved" << endl;
						//start thread
//							pthread_mutex_lock(&runningLock);
//							runningThreads++;
//							pthread_mutex_unlock(&runningLock);
						//spawn threads pass msg, and socket
						pthread_t t;
						threads.push_back(t);
						ThreadArgs *args = new
						ThreadArgs;
						args->cSocket = clientSocket;
						args->msg = executeMsg;
						pthread_create(&threads[threads.size() - 1], NULL,
								&executeThread, (void *) args);
					} else {
						cout << "unknown msg type" << endl;
					}							//else
				}							//if
			} else {
				//listen for terminate message
				Message recvMsg;
				if (recvMsg.receiveMessage(i) <= 0) {	//binder went down?
					close(i);
					cout << "removing" << endl;
					FD_CLR(i, &master);
				} else {
					if (recvMsg.getType() == MSG_TERMINATE) {
						cout << "terminate recieved" << endl;
						terminateFlag = true;
						break;
					}							//msg_terminate
				}							//else
			}							//else
		}							//in read set
	}							//for loop

	if (terminateFlag) {
		break;
	}
}							//while loop

//wait for threads to finish
for (int i = 0; i < threads.size(); i++) {
	/* std::cout << *it; ... */
	pthread_t t = threads[i];
	cout << "joining" << endl;
	pthread_join(t, NULL );
}
//this might not be necessary anymore
//	while (runningThreads > 0) {
//		sleep(1);
//	}
close(clientListenerSocket);
close(binderSocket);
return 0;

}

int rpcTerminate() {
//listen for execute msgs or terminate msgs may need extra threads
//execute stuff, send back success/ failure
cout << "rpcTerminate" << endl;
int clientBinderSocket = createSocket(NULL, NULL );
//send term msg
string dummyMsg = "hi";
Message
terminate(MSG_TERMINATE, dummyMsg);
terminate.sendMessage(clientBinderSocket);
cout << "sentTerminate" << endl;
close(clientBinderSocket);
}


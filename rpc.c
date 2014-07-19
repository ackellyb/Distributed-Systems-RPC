/*
 * rpc.c
 *
 *  Created on: Jul 3, 2014
 *      Author: Aaron
 */

#include "rpc.h"
#include "rpcheader.h"
#include "message.h"
#include "err.h"
#include "common.h"

int binderSocket = -1;
int clientListenerSocket = -1;
int serverPort = -1;
char address[HOST_NAME_SIZE];
map <string, skeleton> localDb;
volatile int runningThreads = 0;
pthread_mutex_t runningLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t localDbLock = PTHREAD_MUTEX_INITIALIZER;

void ** parseArguments(int * argTypes, int len, string argStr, bool isSuccessMessage) {
	stringstream ss( argStr);
	string argTemp;

	void ** args = (void **) malloc(len * sizeof(void *));
	for (int i = 0; i < len; i++) {
		int type = getpType(argTypes[i]);
		int isOutput = isOnlyOutput(argTypes[i]) && !isSuccessMessage;
		int arrayLen = getArrayLen(argTypes[i]);

		if (arrayLen > 0) { //is an array
			if (type == ARG_CHAR) {
				char * array = new char[arrayLen];
				if (!isOutput) {
					for (int j = 0; j < arrayLen; j++) {
						getline(ss, argTemp, ';');
						int c = atoi(argTemp.c_str());
						array[j] = (char)c;
					}
				}
				args[i] = (void*) array;
			} else if (type == ARG_SHORT) {
				short * array = new short[arrayLen];
				if (!isOutput) {
					for (int j = 0; j < arrayLen; j++) {
						getline(ss, argTemp, ';');
						array[j] = (short) atoi(argTemp.c_str());
					}
				}
				args[i] = (void*) array;
			} else if (type == ARG_INT) {
				int * array = new int[arrayLen];
				if (!isOutput) {
					for (int j = 0; j < arrayLen; j++) {
						getline(ss, argTemp, ';');
						array[j] = atoi(argTemp.c_str());
					}
				}
				args[i] = (void*) array;
			} else if (type == ARG_LONG) {
				long * array = new long[arrayLen];
				if (!isOutput) {
					for (int j = 0; j < arrayLen; j++) {
						getline(ss, argTemp, ';');
						array[j] = atol(argTemp.c_str());
					}
				}
				args[i] = (void*) array;
			} else if (type == ARG_FLOAT) {
				float * array = new float[arrayLen];
				if (!isOutput) {
					for (int j = 0; j < arrayLen; j++) {
						getline(ss, argTemp, ';');
						array[j] = toFloat(argTemp);
					}
				}
				args[i] = (void*) array;
			} else if (type == ARG_DOUBLE) {
				double * array = new double[arrayLen];
				if (!isOutput) {
					for (int j = 0; j < arrayLen; j++) {
						getline(ss, argTemp, ';');
						array[j] = toDouble(argTemp);
					}
				}
				args[i] = (void*) array;
			}
			getline(ss, argTemp, '#');
		} else { //not an array
			getline(ss, argTemp, '#');
			if (type == ARG_CHAR) {
				char* arg = new char(0);
				if (!isOutput) {
					int c = atoi(argTemp.c_str());
					*arg = (char)c;
				}
				args[i] = (void*) arg;
			} else if (type == ARG_SHORT) {
				short* arg = new short(0);
				if (!isOutput) {
					*arg = (short) atoi(argTemp.c_str());
				}
				args[i] = (void*) arg;
			} else if (type == ARG_INT) {
				int *arg = new int(0);
				if (!isOutput) {
					*arg = atoi(argTemp.c_str());
				}
				args[i] = (void*) arg;
			} else if (type == ARG_LONG) {
				long* arg = new long(0);
				if(!isOutput) {
					*arg = atol(argTemp.c_str());
				}
				args[i] = (void*) arg;
			} else if (type == ARG_FLOAT) {
				float* arg = new float(0);
				if (!isOutput) {
					*arg = toFloat(argTemp);
				}
				args[i] = (void*) arg;
			} else if (type == ARG_DOUBLE) {
				double* arg = new double(0);
				if (!isOutput) {
					*arg = toDouble(argTemp);
				}
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
	delete []args;

}
void copyArgValues(int * argTypes, int len, void** orgArgs, void** newArgs) {
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
			} else if (type == ARG_DOUBLE) {
				*((double*) orgArgs[i]) = *((double*) newArgs[i]);
			}
		}
	}
}


//Server Function
int rpcInit() {
	char * address = getenv("BINDER_ADDRESS");
	char * port = getenv("BINDER_PORT");
	int retVal;

	//Connecting to binder
	binderSocket = createSocket(address, port);
	if (binderSocket < 0) {
		return binderSocket;
	}

	//Opening connection for clients
	clientListenerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientListenerSocket < 0) {
		return CANT_CREATE_SOCKET_ERR;
	}

	struct sockaddr_in sockAddress;
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = INADDR_ANY;
	sockAddress.sin_port = htons(0);
	retVal = bind(clientListenerSocket, (struct sockaddr *) &sockAddress,
			sizeof(sockAddress));
	if (retVal <0) {
		return BIND_ERR;
	}
	retVal = listen(clientListenerSocket, MAX_CONNECTIONS);
	if (retVal == -1) {
		return LISTEN_ERR;
	}

	struct sockaddr_in sin;
	int addrlen = sizeof(sin);
	getsockname(clientListenerSocket, (struct sockaddr *) &sin,
			(socklen_t*) &addrlen);
	serverPort = ntohs(sin.sin_port);
	if (serverPort <= 0) {
		return SOCKET_ERR;
	}

	return SUCCESS;
}

int rpcRegister(char *name, int *argTypes, skeleton f) {
	//insert into local db
	if (binderSocket < 0) {
		return SOCKET_ERR;
	}
	stringstream ss;
	ss << name;
	//check if it has already been registered if so dont bind...

	int retVal = 0;
	string registerMessage = createRegisterMsg(serverPort, name, argTypes, &retVal);
	if (retVal == HOST_NOT_FOUND_ERR) {
		close(binderSocket);
		return HOST_NOT_FOUND_ERR;
	}
	Message sendMsg(MSG_REGISTER, registerMessage);
	sendMsg.sendMessage(binderSocket);

	//listen for acknlowedgment
	Message recvMsg;
	if(recvMsg.receiveMessage(binderSocket) < SUCCESS){
		close(binderSocket);
		return SOCKET_RECIEVE_ERR;
	}

	if (recvMsg.getType() == MSG_REGISTER_SUCCEESS) {

		string argTypeStr;
		ss.str("");
		ss.str(sendMsg.getMessage());
		getline(ss, argTypeStr, ','); //add
		getline(ss, argTypeStr, ','); //port
		getline(ss, argTypeStr, ','); //name
		getline(ss, argTypeStr, ','); //argType
		string key = getKey(name, argTypeStr);
		pthread_mutex_lock(&localDbLock);
		localDb[key] = f;
		pthread_mutex_unlock(&localDbLock);
		return SUCCESS;
	} else if(recvMsg.getType() == MSG_REGISTER_FAILURE){
		//error?
		return atoi(recvMsg.getMessage());
	} else{
		return UNKNOWN_MSG_ERR;
	}
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


int rpcCall(char* name, int* argTypes, void** args) {
	//connect to binder
	int clientBinderSocket = createSocket(NULL, NULL );
	if (clientBinderSocket < 0) {
		return clientBinderSocket;
	}

	//loc_request to binder
	Message locMsg(MSG_LOC_REQUEST, createLocRequestMsg(name, argTypes));
	locMsg.sendMessage(clientBinderSocket);

	// recieve msg
	Message locRecvMsg;
	if (locRecvMsg.receiveMessage(clientBinderSocket) < SUCCESS) {
		close(clientBinderSocket);
		return SOCKET_RECIEVE_ERR;
	}
	close(clientBinderSocket);

	if (locRecvMsg.getType() == MSG_LOC_SUCCESS) {
		char* serverHostName = strtok(locRecvMsg.getMessage(), ",");
		char* serverPortStr = strtok(NULL, ",");

		//create connection to server
		int serverSocket = createSocket(serverHostName, serverPortStr);
		if (serverSocket <= 0) { //server is not up
			close(serverSocket);
			return SERVER_DOWN_ERR;
		}

		Message serverExecuteMsg(MSG_EXECUTE, createExecuteRequestMsg(name, argTypes, args));
		serverExecuteMsg.sendMessage(serverSocket);

		Message serverReceivedMsg;
		if(serverReceivedMsg.receiveMessage(serverSocket) < SUCCESS){
			close(serverSocket);
			return SOCKET_RECIEVE_ERR;
		}
		close(serverSocket);

		if (serverReceivedMsg.getType() == MSG_EXECUTE_SUCCESS) {
			stringstream ss(serverReceivedMsg.getMessage());
			string name, argTypeStr, argStr;
			getline(ss, name, ',');
			getline(ss, argTypeStr, ',');
			getline(ss, argStr);

			vector<int> argTypesVec = parseArgTypes(argTypeStr);
			int length = argTypesVec.size() - 1;


			void ** newArgs = parseArguments(argTypes, length, argStr, true);
			int* out;
			out = (int*) newArgs[0];
			//delete newArgs....
			copyArgValues(argTypes, length, args, newArgs);
			deleteArgValues(argTypes, length, newArgs);

			return SUCCESS;
		} else {
			string errorCode = serverReceivedMsg.getMessage();
			return atoi(errorCode.c_str());
		}

	} else if (locRecvMsg.getType() == MSG_LOC_FAILURE) { //errored
		string errorCode = locRecvMsg.getMessage();
		return atoi(errorCode.c_str());
	} else {
		return UNKNOWN_MSG_ERR;
	}
	return SUCCESS;
}



void *executeThread(void* tArg) {
//	string msg;
	ThreadArgs * tArgs = (ThreadArgs *) tArg;
	int cSocket = tArgs->cSocket;
	Message* msg = tArgs->msg;

	//parse stuff here
	stringstream ss;
	ss << msg->getMessage();
	string name, argTypeStr, argStr;
	getline(ss, name, ',');
	getline(ss, argTypeStr, ',');
	getline(ss, argStr);

	vector<int> argTypesVec = parseArgTypes(argTypeStr);
	int * argTypes = vectorToArray(argTypesVec);
	int length = argTypesVec.size() - 1;

	void ** argArray = parseArguments(argTypes, length, argStr, false);

	string key = getKey(name, argTypeStr);
	pthread_mutex_lock(&localDbLock);
	skeleton f = localDb[key];
	pthread_mutex_unlock(&localDbLock);
	if (f != NULL ) {
		int * out = (int*) argArray[0];
		int retVal = f(argTypes, argArray);
		out = (int*) argArray[0];
		string skelMsg;
		int type;
		if (retVal == 0) {
			char * cstr = new char[name.length()+1];
			strcpy(cstr, name.c_str());
			skelMsg = createExecuteSuccessMsg(cstr, argTypes, argArray);
			type = MSG_EXECUTE_SUCCESS;
			delete[] cstr;
		} else {
			skelMsg = createCodeMsg(SKELETON_EXECUTE_FAIL);
			type = MSG_EXECUTE_FAILURE;
		}
	Message executeSkel( type, skelMsg);
	executeSkel.sendMessage(cSocket);
	deleteArgValues(argTypes, length, argArray);
	} else {
		string skelMsg = createCodeMsg(SKELETON_NOT_FOUND_ERR);
		Message executeFail(MSG_EXECUTE_FAILURE, skelMsg);
		executeFail.sendMessage(cSocket);
		deleteArgValues(argTypes, length, argArray);
	}

	//run skeleton and reply to client using cSocket
	delete [] argTypes;
	delete msg;
	delete tArgs;
	close(cSocket);

}

int rpcExecute() {

	if (localDb.empty()) {
		return NOTHING_REGISTERED_ERR;
	}
	fd_set master, read_fds;
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
						Message *executeMsg = new Message();
						if(executeMsg->receiveMessage(clientSocket) != SUCCESS){
							close(clientSocket);
							return SOCKET_RECIEVE_ERR;
						}

						if (executeMsg->getType() == MSG_EXECUTE) {
							//start thread
							//spawn threads pass msg, and socket
							pthread_t t;
							threads.push_back(t);
							ThreadArgs *args = new
							ThreadArgs;
							args->cSocket = clientSocket;
							args->msg = executeMsg;
							pthread_create(&threads[threads.size() - 1], NULL,
									&executeThread, (void *) args);
						}
					}
				} else {
					//listen for terminate message
					Message recvMsg;
					if (recvMsg.receiveMessage(i) != SUCCESS) {	//binder went down?
						close(i);
						FD_CLR(i, &master);
					} else {
						if (recvMsg.getType() == MSG_TERMINATE) {
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
		pthread_join(t, NULL );
	}
	//this might not be necessary anymore
	close(clientListenerSocket);
	close(binderSocket);
	return SUCCESS;

}

int rpcTerminate() {
	//listen for execute msgs or terminate msgs may need extra threads
	//execute stuff, send back success/ failure
	int clientBinderSocket = createSocket(NULL, NULL );
	//send term msg
	Message terminate(MSG_TERMINATE);
	terminate.sendMessage(clientBinderSocket);
	close(clientBinderSocket);
	return SUCCESS;
}


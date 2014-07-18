/*
 * rpcheader.h
 *
 *  Created on: Jul 18, 2014
 *      Author: Aaron
 */

#ifndef RPCHEADER_H_
#define RPCHEADER_H_


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
#include "message.h"

#define MAX_CONNECTIONS 100
#define HOST_NAME_SIZE 256

using namespace std;

class ThreadArgs {
public: int cSocket;
	Message* msg;
};




#endif /* RPCHEADER_H_ */

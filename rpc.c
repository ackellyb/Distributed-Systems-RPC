/*
 * rpc.c
 *
 *  Created on: Jul 3, 2014
 *      Author: Aaron
 */


#include <rpc.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAX_CONNECTIONS 100
#define HOST_NAME_SIZE 256

int binderSocket, clientSocket, serverPort;
char address[HOST_NAME_SIZE];

//Server Function
int rpcInit() {
	char * address = getenv("BINDER_ADDRESS");
	char * port = getenv("BINDER_PORT");
	int retVal;

	if (address == NULL || port == NULL) {
		return -1;
	}


	//Connecting to binder
	binderSocket = socket(AF_INET , SOCK_STREAM , 0);
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
	if(clientSocket == -1) {
		return -1;
	}

	struct sockaddr_in sockAddress;
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = INADDR_ANY;
	sockAddress.sin_port = htons(0);
	retVal = bind(clientSocket , (struct sockaddr *)&sockAddress, sizeof(sockAddress));
	if (retVal == -1) {
		return -1;
	}
	retVal = listen (clientSocket, MAX_CONNECTIONS);
	if(retVal == -1) {
		return -1;
	}

	struct sockaddr_in sin;
	int addrlen = sizeof(sin);
	getsockname(clientSocket, (struct sockaddr *)&sin, (socklen_t*)&addrlen);
	int serverPort = ntohs(sin.sin_port);
	if (serverPort <= 0) {
		return -1;
	}
	gethostname(address, HOST_NAME_SIZE);
	if (address == NULL) {
		return -1;
	}

	return SUCCESS;
}




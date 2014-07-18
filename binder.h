/*
 * Binder.h
 *
 *  Created on: Jul 18, 2014
 *      Author: Aaron
 */

#ifndef BINDER_H_
#define BINDER_H_

#include <stdlib.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <algorithm>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <map>
#include <vector>
#include <utility>
#include <set>
#include <string>
#include <stdio.h>

using namespace std;

static int MAXNUMBER = 100;

class Server {
	char* host;
	int port;
	int usedCounter;
public:
	Server(string h, int p);
	~Server();
	int getPort();
	string getHost();
	string getIdentifier();
	void incrementCounter();
	int getCounter();

	bool operator>(const Server &s2);
	bool operator==(const Server &s2);
	bool operator<(const Server &s2);
};




#endif /* BINDER_H_ */

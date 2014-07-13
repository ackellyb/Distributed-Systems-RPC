/*
 * binder.cc
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */
#include <string>
#include <stdio.h>
#include "message.h"
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
using namespace std;
int MAXNUMBER = 100;
//change this wen we know how to...
int MAX_SIZE = 1000;

class Server {
	string host;
	int port;
	int usedCounter;

public:
	Server(string h, int p) {
		host = h;
		port = p;
		usedCounter = 0;
	}
	int getPort() {
		return port;
	}
	string getHost() {
		return host;
	}
	string getIdentifier(){
		stringstream ss;
		ss<<host<<port;
		return ss.str();
	}
	void incrementCounter() {
		usedCounter++;
	}
	int getCounter() {
		return usedCounter;
		}

	bool operator>(const Server &s2) {
		return usedCounter > s2.usedCounter;
	}
	bool operator==(const Server &s2) {
		return usedCounter == s2.usedCounter;
	}
	bool operator<(const Server &s2) {
		return usedCounter < s2.usedCounter;
	}
};

bool compareServers (Server* i,Server* j) {
	return (*i)<(*j);
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

string convertToString(char* c) {
	stringstream ss;
	string s;
	ss < c;
	ss >> s;
	return s;
}

int get_connection(int s) {
	int t;
	if ((t = accept(s, NULL, NULL )) < 0) { /* accept connection if there is one */
		return (-1);
	}
	return (t);
}

int main() {
	int listener = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sockAddress;
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = INADDR_ANY;
	sockAddress.sin_port = htons(0);
	bind(listener, (struct sockaddr *) &sockAddress, sizeof(sockAddress));
	struct sockaddr_in sin;
	int addrlen = sizeof(sin);
	getsockname(listener, (struct sockaddr *) &sin, (socklen_t*) &addrlen);
	int port = ntohs(sin.sin_port);
	char address[256];
	gethostname(address, 256);
	listen(listener, MAXNUMBER);
	//lock this shit up later
	map<string, vector<Server*>  *> dataBase;
	map<string, Server*> servers;

	cout << "BINDER_ADDRESS " << address << endl;
	cout << "BINDER_PORT " << port << endl;
	fd_set master;
	fd_set read_fds;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	int fdmax;
	int newfd;
	fdmax = listener;
	FD_SET(listener, &master);
	stringstream ss;
//	cout << "somthin connected " << port << endl;
	while (true) {
		read_fds = master;
		select(fdmax + 1, &read_fds, NULL, NULL, NULL );
		for (int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == listener) {
					//process new connection
					newfd = get_connection(listener);
					FD_SET(newfd, &master);
					if (newfd > fdmax) {
						fdmax = newfd;
					}
				} else {
					//process data
					char incommingMsg[MAX_SIZE];
					if (recv(i, incommingMsg, MAX_SIZE, 0) <= 0) {
						close(i);
						FD_CLR(i, &master);
					} else {
						//parse messsage
						cout << "somthin connected "  << endl;
						cout<<incommingMsg<<endl;
						char* length;
						char* commandStr;
//						length = strtok(incommingMsg, ",");
						commandStr = strtok(incommingMsg, ",");
						string stemp;
//	            			ss.str("");
//	            			ss<<incommingMsg;
//	            			string incommingMsgStr = ss.str();
//	            			m = parse_message(incommingMsg);
						string reply;
						int command = atoi(commandStr);
						cout << command << endl;
						if (command == MSG_REGISTER) { //register
							cout << "register recieved "  << endl;
							string hostStr, portStr, nameStr, argTypeStr;
							hostStr = strtok(NULL, ","); //host
							portStr = strtok(NULL, ","); //port
							Server *server;
							server = new Server(hostStr, atoi(portStr.c_str()));
							if(servers.count(server->getIdentifier()) > 0){//get same server object if exists
								server = servers[server->getIdentifier()];
							}else{//create a new one, add it to servers as well
								servers[server->getIdentifier()] = server;
							}
							cout<<"still alive " <<endl;
							nameStr = strtok(NULL, ","); //name
							argTypeStr = strtok(NULL, ","); //argTypes array
							string key = getKey(nameStr, argTypeStr);
							if (dataBase.count(key) > 0) { //if exists add to existing vector
								vector<Server* > *v = dataBase[key];
								v->push_back(server);
								dataBase[key] = v;

							} else { //create new vector and insert it
								vector<Server* > *v = new vector<Server* >;
								v->push_back(server);
								dataBase[key] = v;
							}

							//when does register fail?
							reply = createRegisterSuccessMsg();

						}else if (command == MSG_LOC_REQUEST) { //loc_request
							cout << "location request recieved "  << endl;
							string nameStr, argTypeStr;
							nameStr = strtok(NULL, ","); //name
							argTypeStr = strtok(NULL, ","); //argTypes
							string key = getKey(nameStr, argTypeStr);
							if (dataBase.count(key) > 0) {
								//sort list by count
								//increase counter
								vector <Server* > *v = dataBase[key];
								//sort list based on counter
								sort (v->begin(), v->end(), compareServers);
								Server *s = v->front();
								s->incrementCounter();

								reply = createLocSuccessMsg(s->getHost(),
										s->getPort());
								//put it back into queue
							} else {
								//return fail signature doesnt exist
								//temp error code

								reply = createFailureMsg(MSG_LOC_FAILURE, -1);
							}
						} else if (command == MSG_TERMINATE) {//terminate
							//kill all servers in db, send terminate to all of them
							//not sure how to do this...., need another map to track all registerd servers?
							//or send to all connected?
							//close all sockets
//	            				return;
						} else {
							//unsporrted message type?
							cout << "HUH?! "  << endl;
//						reply = createFailureMsg(,-1);
					}
					//send data
					int len = strlen(reply.c_str()) + 1;
					send(i, reply.c_str(), len, 0);

				}	            				//else
			}	            				//data
		}	            				//process readable fd
	}	            				//for loop
}	            				//infinite for loop

//delete db?
}


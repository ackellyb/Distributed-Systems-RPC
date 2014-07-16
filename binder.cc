/*
 * binder.cc
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */
#include <string>
#include <stdio.h>
#include "message.h"
#include "common.h"
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
using namespace std;
int MAXNUMBER = 100;

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
	string getIdentifier() {
		stringstream ss;
		ss << host << port;
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

bool compareServers(Server* i, Server* j) {
	return (*i) < (*j);
}

void cleanUpDb(){
//iterate through map
	//iterate through list
	//delete all servers
	//delete all lists
	//delete map
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
	map<string, vector<Server*> *> dataBase;
	map<string, Server*> servers;

	cout << "BINDER_ADDRESS " << address << endl;
	cout << "BINDER_PORT " << port << endl;
	fd_set master;
	fd_set read_fds;
//	fd_set server_fds;
	set<int> serverFds;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
//	FD_ZERO(&server_fds);
	int fdmax;
	int newfd;
	fdmax = listener;
	FD_SET(listener, &master);
	stringstream ss;
	bool terminated = false;

	while (true) {
		if(terminated && serverFds.empty()){
			close(listener);
			cleanUpDb();
			cout<<"all done"<<endl;
			return 0;
		}
		read_fds = master;
		select(fdmax + 1, &read_fds, NULL, NULL, NULL );
		for (int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == listener && !terminated) {//if terminated dont accept any new requests
					//process new connection
					newfd = get_connection(listener);
					FD_SET(newfd, &master);
//					FD_SET(newfd, &server_fds);
					serverFds.insert(newfd);
					cout<< "new connection: "<<newfd<<endl;
					if (newfd > fdmax) {
						fdmax = newfd;
					}
				} else {
					//process data
					Message recvMsg;
					if (recvMsg.receiveMessage(i) <= 0) {
						close(i);
						cout<<"removing "<< i <<endl;
						FD_CLR(i, &master);
//						FD_CLR(i, &server_fds);
						serverFds.erase(i);

					} else {
						//parse messsage
						cout << "somthin connected "  << endl;
						cout << "Message" << recvMsg.getMessage() << endl;
						string reply;
						int type;
						cout << recvMsg.getType() <<  " " << recvMsg.getTypeString() << endl;

						if (recvMsg.getType()  == MSG_REGISTER) { //register

							cout << "register recieved "  << endl;
							string hostStr, portStr, nameStr, argTypeStr;
							hostStr = strtok(recvMsg.getMessage(), ","); //host
							portStr = strtok(NULL, ","); //port
							Server *server;
							server = new Server(hostStr, atoi(portStr.c_str()));
							if (servers.count(server->getIdentifier()) > 0) { //get same server object if exists
								server = servers[server->getIdentifier()];
							} else {//create a new one, add it to servers as well
								servers[server->getIdentifier()] = server;
							}
							cout << "still alive " << endl;
							nameStr = strtok(NULL, ","); //name
							argTypeStr = strtok(NULL, ","); //argTypes array
							string key = getKey(nameStr, argTypeStr);
							if (dataBase.count(key) > 0) { //if exists add to existing vector
								vector<Server*> *v = dataBase[key];
								v->push_back(server);
								dataBase[key] = v;

							} else { //create new vector and insert it
								vector<Server*> *v = new vector<Server*>;
								v->push_back(server);
								dataBase[key] = v;
							}

							//when does register fail?
							reply = createCodeMsg(0);
							type = MSG_REGISTER_SUCCEESS;

						} else if (recvMsg.getType() == MSG_LOC_REQUEST) { //loc_request
							cout << "location request recieved "  << endl;
//							FD_CLR(i, &server_fds);
							serverFds.erase(i);
							string nameStr, argTypeStr;
							nameStr = strtok(recvMsg.getMessage(), ","); //name
							argTypeStr = strtok(NULL, ","); //argTypes
							string key = getKey(nameStr, argTypeStr);
							if (dataBase.count(key) > 0) {
								//sort list by count
								//increase counter
								vector<Server*> *v = dataBase[key];
								//sort list based on counter
								sort(v->begin(), v->end(), compareServers);
								Server *s = v->front();
								s->incrementCounter();

								reply = createLocSuccessMsg(s->getHost(),
										s->getPort());
								type = MSG_LOC_SUCCESS;
								//put it back into queue
							} else {
								//return fail signature doesnt exist
								//temp error code

								reply = createCodeMsg(-1);
								type = MSG_LOC_FAILURE;
							}

						} else if (recvMsg.getType() == MSG_TERMINATE) {//terminate
							//kill all servers in db, send terminate to all of them
							//not sure how to do this...., need another map to track all registerd servers?
							//or send to all connected?
							//close all sockets
//							FD_CLR(i, &server_fds);
							serverFds.erase(i);
							string dumyMsg= "hi";
							Message terminate(MSG_TERMINATE, dumyMsg);

							int len = strlen(reply.c_str()) + 1;
							cout << "listener: "<<listener << endl;
							cout << "Servers connected: "<<fdmax << endl;
							for (int j = 0; j <= fdmax; j++) {
								//sned terminate to all servers
								cout << j << endl;
								if(serverFds.find(j) != serverFds.end()) {
									try {
										cout << "sending terminate to servers"
												<< endl;
										terminate.sendMessage(j);
										cout << "sent to "<< j << endl;
									} catch (...) {
										cout << "ignoring error" << endl;
									}
								}
							}
							terminated = true;
						} else {
							//unsporrted message type?
							cout << "HUH?! "  << endl;
//							reply = createFailureMsg(,-1);
					}
					//send data
					Message replyMsg(type, reply);
					replyMsg.sendMessage(i);

					}	            				//else
				}	            				//data
			}	            				//process readable fd
		}	            				//for loop
	}	            				//infinite for loop

//delete db?
}


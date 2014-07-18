/*
 * binder.cc
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */

#include "message.h"
#include "common.h"
#include "err.h"
#include "binder.h"

Server::Server(string h, int p) {
	host = new char[h.length()+1];
	strcpy(host, h.c_str());
	port = p;
	usedCounter = 0;
}

Server::~Server() {
	delete [] host;
}

int Server::getPort() {
	return port;
}

string Server::getHost() {
	return host;
}

string Server::getIdentifier() {
	stringstream ss;
	ss << host << port;
	return ss.str();
}

void Server::incrementCounter() {
	usedCounter++;
}

int Server::getCounter() {
	return usedCounter;
}

bool Server::operator>(const Server &s2) {
	return usedCounter > s2.usedCounter;
}

bool Server::operator==(const Server &s2) {
	return usedCounter == s2.usedCounter;
}

bool Server::operator<(const Server &s2) {
	return usedCounter < s2.usedCounter;
}


bool compareServers(Server* i, Server* j) {
	return (*i) < (*j);
}



// Removes all references to servers, and their skeleton associations
void cleanUpDb(map<string, vector<Server*> *> &dataBase, map<string, Server*> &servers ) {
	//iterate through map
	vector<Server*> * serverList;
	Server* server;
	for (map<string, vector<Server*> *>::iterator it = dataBase.begin();
			it != dataBase.end(); ++it) {//delete all vectors
		serverList = it->second;
		serverList->clear();
		delete serverList;
		//delete all lists
	}
	for (map<string, Server*>::iterator it = servers.begin();
				it != servers.end(); ++it) {//delete all servers
		delete it->second;
	}
}


int main() {
	int listener = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sockAddress;
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = INADDR_ANY;
	sockAddress.sin_port = htons(0);

	int retVal = bind(listener, (struct sockaddr *) &sockAddress, sizeof(sockAddress));
	if (retVal != SUCCESS) {
		return BIND_ERR;
	}

	struct sockaddr_in sin;
	int addrlen = sizeof(sin);
	retVal = getsockname(listener, (struct sockaddr *) &sin, (socklen_t*) &addrlen);
	if (retVal != SUCCESS) {
		return SOCKET_NAME_FAIL;
	}

	int port = ntohs(sin.sin_port);
	char address[256];
	retVal = gethostname(address, 256);
	if (retVal != SUCCESS) {
		return HOST_NOT_FOUND;
	}

	retVal = listen(listener, MAXNUMBER);
	if(retVal != SUCCESS) {
		return LISTEN_ERR;
	}

	map<string, vector<Server*> *> dataBase;
	map<string, Server*> servers;

	cout << "BINDER_ADDRESS " << address << endl;
	cout << "BINDER_PORT " << port << endl;
	fd_set master, read_fds;
	set<int> serverFds;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	int fdmax, newfd;
	fdmax = listener;
	FD_SET(listener, &master);
	stringstream ss;
	bool terminated = false;

	while (true) {
		if (terminated && serverFds.empty()) {
			close(listener);
			cleanUpDb(dataBase, servers);
			return 0;
		}
		read_fds = master;
		select(fdmax + 1, &read_fds, NULL, NULL, NULL );
		for (int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == listener && !terminated) { //if terminated dont accept any new requests
					//process new connection
					newfd = getConnection(listener);
					FD_SET(newfd, &master);
					serverFds.insert(newfd);
					if (newfd > fdmax) {
						fdmax = newfd;
					}
				} else {
					//process data
					Message recvMsg;
					if (recvMsg.receiveMessage(i) != SUCCESS) {
						close(i);
						FD_CLR(i, &master);
						serverFds.erase(i);

					} else {
						//parse messsage
						string reply;
						int type;

						if (recvMsg.getType() == MSG_REGISTER) { //register

							string hostStr, portStr, nameStr, argTypeStr;
							hostStr = strtok(recvMsg.getMessage(), ","); //host
							portStr = strtok(NULL, ","); //port

							Server *server;
							Server serverTemp = Server(hostStr, atoi(portStr.c_str()));

							if (servers.count(serverTemp.getIdentifier()) > 0) { //get same server object if exists
								server = servers[server->getIdentifier()];
							} else { //create a new one, add it to servers as well
								server = new Server(hostStr, atoi(portStr.c_str()));
								servers[server->getIdentifier()] = server;
							}

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

							reply = createCodeMsg(0);
							type = MSG_REGISTER_SUCCEESS;

						} else if (recvMsg.getType() == MSG_LOC_REQUEST) { //loc_request
							serverFds.erase(i);
							string nameStr, argTypeStr;
							nameStr = strtok(recvMsg.getMessage(), ","); //name
							argTypeStr = strtok(NULL, ","); //argTypes
							string key = getKey(nameStr, argTypeStr);

							if (dataBase.count(key) > 0) {
								//sort list by count
								vector<Server*> *v = dataBase[key];
								//sort list based on counter
								sort(v->begin(), v->end(), compareServers);
								Server *s = v->front();
								s->incrementCounter();

								reply = createLocSuccessMsg(s->getHost(),
										s->getPort());
								type = MSG_LOC_SUCCESS;
							} else {
								reply = createCodeMsg(-1);
								type = MSG_LOC_FAILURE;
							}

						} else if (recvMsg.getType() == MSG_TERMINATE) {//terminate
							//kill all servers in db, send terminate to all of them

							serverFds.erase(i);
							Message terminate(MSG_TERMINATE);

							int len = strlen(reply.c_str()) + 1;
							for (int j = 0; j <= fdmax; j++) {
								//send terminate to all servers
								if (serverFds.find(j) != serverFds.end()) {
									try {
										terminate.sendMessage(j);
									} catch (...) {
									}
								}
							}
							terminated = true;
						}
						//send data
						if (!terminated) {
							Message replyMsg(type, reply);
							replyMsg.sendMessage(i);
						}

					}	            				//else
				}	            				//data
			}	            				//process readable fd
		}	            				//for loop
	}	            				//infinite for loop

}

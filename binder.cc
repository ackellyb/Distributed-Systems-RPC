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
#include <queue>
#include <utility>
using namespace std;
int MAXNUMBER=100;
int MAX_SIZE = 2^32-1;

string getKey(string name, string argTypeStr){
	stringstream ss;
	stringstream ssOut;
	ss.str(argTypeStr);
	ssOut <<name<<",";
	string stemp;
	int withOutLen;
	int removeLen = 65535 << 16; //16 1s followed by 16 0s
	while(getline(ss, stemp, '#')){
		int withLen = atoi(stemp.c_str());
			//elemenate lower 16 bits
			withOutLen = withLen & removeLen;
		if (withLen != withOutLen){
			//mark as array
			withOutLen = withOutLen | 1;
		}
			ssOut << withLen << "#";
	}
	return ssOut.str();
}

string convertToString(char* c){
	stringstream ss;
	string s;
	ss<c;
	ss>>s;
	return s;
}


int get_connection(int s) {
	int t;
	if ((t = accept(s,NULL,NULL)) < 0){ /* accept connection if there is one */
		return(-1);
	}
	return(t);
}

int main(){
	int listener = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sockAddress;
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = INADDR_ANY;
	sockAddress.sin_port = htons(0);
	bind(listener , (struct sockaddr *)&sockAddress, sizeof(sockAddress));
	struct sockaddr_in sin;
	int addrlen = sizeof(sin);
	getsockname(listener, (struct sockaddr *)&sin, (socklen_t*)&addrlen);
	int port = ntohs(sin.sin_port);
	char address[256];
	gethostname(address, 256);
	listen (listener, MAXNUMBER);
	//lock this shit up later
	map <string,  queue< pair <string, int> *> * > dataBase;

	cout << "SERVER_ADDRESS " << address <<endl;
	cout << "SERVER_PORT " << port << endl;
	fd_set master;
	    fd_set read_fds;
	    FD_ZERO(&master);
	    FD_ZERO(&read_fds);
	    int fdmax;
	    int newfd;
	    fdmax = listener;
	    FD_SET(listener, &master);
	    stringstream ss;
		while(true){
			read_fds = master;
		  	select(fdmax+1, &read_fds, NULL, NULL, NULL);
		  	for(int i = 0; i <= fdmax; i++){
		   		if (FD_ISSET(i, &read_fds)){
		     		if (i == listener){
		     			//process new connection
		     			newfd = get_connection(listener);
		     			FD_SET(newfd, &master);
		     			if (newfd > fdmax){
	            			fdmax = newfd;
	            		}
		     		}else{
		     			//process data
		    			char incommingMsg[MAX_SIZE];
		     			if (recv(i, incommingMsg, MAX_SIZE, 0) <= 0){
	            			close(i);
	                		FD_CLR(i, &master);
	            		}else{
	            			//parse messsage
	            			 char* length;
	            			 char* command;
	            			 length = strtok(incommingMsg, ",");
	            			 command = strtok(NULL, ",");
	            			 string stemp;
//	            			ss.str("");
//	            			ss<<incommingMsg;
//	            			string incommingMsgStr = ss.str();
//	            			m = parse_message(incommingMsg);
	            			string reply;

	            			if(strcmp(command, "register")== 0 ){//register
	            				string hostStr, portStr, nameStr, argTypeStr;
	            				hostStr = strtok(NULL, ","); //host
	            				portStr = strtok(NULL, ",");//port
	            				pair<string, int> *location = new pair<string, int>;
	            				location->first = hostStr;
	            				location->second = atoi(portStr.c_str());

	            				nameStr = strtok(NULL, ",");//name
	            				argTypeStr = strtok(NULL, ",");//argTypes array
	            				string key = getKey(nameStr, argTypeStr);
	            				if(dataBase.count(key) >0){//if exists add to existing queue
	            					queue <pair <string, int>* > *q = dataBase[key];
	            					q->push(location);
//	            					dataBase[key] =  q ;
	            				}else{//create new queue and insert it
	            					queue <pair <string, int> *>* q = new queue <pair <string, int> *>;
	            					q->push(location);
	            					dataBase[key]= q ;
	            				}

	            				//send success
	            				reply = createSuccessMsg();

	            			}else if(strcmp(command, "loc_request")== 0 ){//loc_request
	            				//query db
	            				//find queue
	            				//remove first
	            				//add first back into queue
	            			}else if(strcmp(command, "terminate")== 0 ){//terminate
	            				//kill all servers in db
	            				//close all sockets
	            				//return
	            			}
	            			//send data
	            			int len = strlen(reply.c_str())+1;
	            			send(i, reply.c_str(), len, 0);

						}//else
		    		}//data
		    	}//process readable fd
			}//for loop
		}//infinite for loop


		//delete db?
}


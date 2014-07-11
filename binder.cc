/*
 * binder.cc
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <algorithm>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
using namespace std;
int MAXNUMBER=100;
int MAX_SIZE = 2^32-1;

message parse_message(char[] msg){
message
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
	            			message m;
	            			m = parse_message(incommingMsg);

	            			if(true){//register
	            				//insert into db

	            			}else if(false){//loc_request
	            				//query db
	            			}else if(false){//terminate
	            				//kill all servers in db
	            			}
	            			//send data


						}//else
		    		}//data
		    	}//process readable fd
			}//for loop
		}//infinite for loop

}




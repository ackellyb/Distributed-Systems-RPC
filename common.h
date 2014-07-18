/*
 * Common.h
 *
 *  Created on: Jul 15, 2014
 *      Author: Aaron
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <string>
#include <sstream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>


using namespace std;

int getArrayLen(int type);
int getpType(int type);
bool isOnlyOutput(int type);
string toHex(unsigned char * array, int len);
string dToHex(double x);
string flToHex(float x);
double toDouble(string x);
float toFloat(string x);
string getKey(string name, string argTypeStr);
int * vectorToArray(vector<int> vec);
string convertToString(char* c);
int getConnection(int s);
int createSocket(char* addr, char* prt);

#endif /* COMMON_H_ */

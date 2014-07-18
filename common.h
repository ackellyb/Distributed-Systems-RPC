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
void printDEBUG(string s, pthread_mutex_t lock);

#endif /* COMMON_H_ */

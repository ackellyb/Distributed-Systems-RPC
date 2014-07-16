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

using namespace std;

int getArrayLen(int type);
int getpType(int t);
string toHex(unsigned char * array, int len);
string dToHex(double x);
string flToHex(float x);
double toDouble(string x);
float toFloat(string x);

#endif /* COMMON_H_ */

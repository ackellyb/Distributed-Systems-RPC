/*
 * message.h
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */

#include <string>
#ifndef MESSAGE_H_
#define MESSAGE_H_

#define MSG_REGISTER      1
#define MSG_LOC_REQUEST   2


#define MSG_TERMINATE     3
using namespace std;


string createRegisterMsg(int port, char *name, int *argTypes);
string createSuccessMsg();
string createTerminate();

#endif /* MESSAGE_H_ */

/*
 * err.h
 *
 *  Created on: Jul 7, 2014
 *      Author: Aaron
 */

#ifndef ERR_H_
#define ERR_H_

#define SUCCESS 0

#define CANT_CREATE_SOCKET_ERR 	-1
#define CANT_CONNECT_SOCKET_ERR -2
#define SOCKET_ERR -3
#define SOCKET_RECIEVE_ERR -4

#define SERVER_DOWN_ERR -5
#define SKELETON_EXECUTE_FAIL -6
#define SKELETON_NOT_FOUND_ERR -7
#define UNKNOWN_MSG_ERR -8

#define NOTHING_REGISTERED_ERR -9

#define MESSAGE_SEND_ERR -10
#define MESSAGE_RECEIVE_ERR -11
#define MESSAGE_CREATION_FAIL -12

#define HOST_NOT_FOUND -13
#define BIND_ERR -14
#define SOCKET_NAME_FAIL -15;
#define LISTEN_ERR -16;




#endif /* ERR_H_ */

/*
 * message.cc
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */
#include "message.h"
#include <sstream>
#include <cstring>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

//commas separate args, # separates array elements
//length, msgCode, hostname, port, name, argTypes
 string createRegisterMsg(int port, char *name, int *argTypes){
	  char address[256];
	  gethostname(address, 256);
	  stringstream ss;
	  ss << "1," << address << ","<<  port<< "," << name << ",";
	  int lengthArray= sizeof(argTypes)/sizeof(*argTypes);
	  for(int i = 0; i<= lengthArray; i++){
	    ss << argTypes[i]<< "#";
	  }
	  string msg;
	  msg = ss.str();
	  int len = strlen(msg.c_str())+1;
	  ss.str("");
	  ss << len <<"," << msg;
	  msg = ss.str();
	  cout<< msg << endl;
	  return msg;
}


 string createSuccessMsg(){
	return "0";
}

 string createRegisterFailtureMsg(int reasoncode){

}


// string createExecuteMsg(char *name, int *argTypes){//args
//	 stringstream ss;
//	 	  ss << "3," << name << ",";
//	 	  int lengthArray= sizeof(argTypes)/sizeof(*argTypes);
//	 	  for(int i = 0; i<= lengthArray; i++){
//	 	    ss << argTypes[i]<< "#";
//	 	  }
//	 	  ss<< ", ";
//
//	 	 lengthArray= sizeof(args)/sizeof(*args);
//	 	 	 	  for(int i = 0; i<= lengthArray; i++){
//	 	 	 	    ss << args[i]<< "#";
//	 	 	 	  }
//
//	 	  string msg;
//	 	  msg = ss.str();
//	 	  int len = strlen(msg.c_str())+1;
//	 	  ss.str("");
//	 	  ss << len <<"," << msg;
//	 	  msg = ss.str();
//	 	  cout<< msg << endl;
//	 	  return msg;
//}
//
// string createExecuteSuccessMsg(char *name, int *argTypes){//args
//	 stringstream ss;
//	 	  ss << "1," << address << ","<<  port<< "," << name << ",";
//	 	  int lengthArray= sizeof(argTypes)/sizeof(*argTypes);
//	 	  for(int i = 0; i<= lengthArray; i++){
//	 	    ss << argTypes[i]<< "#";
//	 	  }
//	 	  string msg;
//	 	  msg = ss.str();
//	 	  int len = strlen(msg.c_str())+1;
//	 	  ss.str("");
//	 	  ss << len <<", " << msg;
//	 	  msg = ss.str();
//	 	  cout<< msg << endl;
//	 	  return msg;
//}

 string createExecuteFailureMsg(int reasonCode){
}

 string createTerminate(){
	 return "-454";
}

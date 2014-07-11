/*
 * message.cc
 *
 *  Created on: Jul 11, 2014
 *      Author: stephen
 */
#include <message.h>
#include <sstream>
#include <cstring>

 String createRegisterMsg(int port, char *name, int *argTypes){
	  char address[256];
	  gethostname(address, 256);
	  stringstream ss;
	  ss << "1, " << address << ", "<<  port<< ", " << name << ", ";
	  int lengthArray= sizeof(argTypes)/sizeof(*argTypes);
	  for(int i = 0; i<= lengthArray; i++){
	    ss << argTypes[i]<< " ";
	  }
	  string msg;
	  msg = ss.str();
	  int len = strlen(msg.c_str())+1;
	  ss.str("");
	  ss << len <<", " << msg;
	  msg = ss.str();
	  cout<< msg << endl;
	  return msg;
}


 String createSuccessMsg(){
	return "0";
}

 String createRegisterFailtureMsg(int reasoncode){

}


 String createExecuteMsg(){
}

 String createExecuteSuccessMsg(){
}

 String createExecuteFailureMsg(){
}

 String createTerminate(){
}




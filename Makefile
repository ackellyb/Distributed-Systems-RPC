run:
	g++ -c common.h common.cc
	g++ -c rpc.h  rpcheader.h rpc.c
	g++ -c message.h message.cc
	ar -cvq librpc.a rpc.o message.o common.o
	ranlib librpc.a 
	g++ -c binder.h binder.cc
	g++ -o binder message.o binder.o common.o
	
test:
	g++ -c rpc.h client1.c 
	g++ -L. client1.o  -lrpc -pthread -o client
	g++ -c rpc.h server.c
	g++ -c server_function_skels.c  
	g++ -c server_functions.c  
	g++ -L. server_functions.o server_function_skels.o server.o -lrpc -pthread -o server
	
clean:
	rm *.o
	rm *.a
	rm *.gch
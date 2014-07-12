run:
	g++ -c rpc.h rpc.c
	g++ -c message.h message.cc
	ar -cvq librpc.a message.o rpc.o
	ranlib librpc.a
	g++ -c binder.cc
	g++ -o binder message.o binder.o
	
test:
	g++ -c client1.c 
	g++ -c server.c
	g++ -c server_function_skels.c  
	g++ -c server_functions.c  
	g++ -L. client1.o -lrpc -o client
	g++ -L. server_functions.o server function_skels.o server.o -lrpc -o server
	
clean:
	rm *.o
	rm *.gch
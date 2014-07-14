run:
	g++ -c rpc.h rpc.c
	g++ -c message.h message.cc
	ar -cvq librpc.a rpc.o message.o
	ranlib librpc.a 
	g++ -c binder.cc
	g++ -o binder message.o binder.o
	
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
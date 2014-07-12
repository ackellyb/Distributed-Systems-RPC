run:
	g++ -c rpc.h rpc.c
	ar -cvq librpc.a rpc.o
	ranlib librpc.a
	g++ -c message.h message.cc
	g++ -c binder.cc
	g++ -o binder message.o binder.o
	
test:
	g++ client1.c -L. -lrpc
	g++ server.c -L. -lrpc
	
clean:
	rm *.o
	rm *.gch
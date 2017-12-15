all:
	g++ server.cpp response.h -o server
	g++ client.cpp request.h response.h -o client
clean:
	rm -f *~client server
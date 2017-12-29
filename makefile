all:
	g++ server.cpp response.h parsing.h -o server -lcrypto -g
	g++ client.cpp request.h response.h -o client -lcrypto -g
clean:
	rm -f *~client server
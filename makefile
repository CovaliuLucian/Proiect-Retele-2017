all:
	g++ server.cpp response.h -o server -lcrypto
	g++ client.cpp request.h response.h -o client -lcrypto
clean:
	rm -f *~client server
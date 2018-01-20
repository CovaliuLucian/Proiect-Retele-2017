#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <cstring>

#include "request.h"
#include "response.h"

#include <iostream>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <string>
#include <iomanip>
#include <sstream>

#include "Crypto.h"
#include <signal.h>

using namespace std;

/* portul de conectare la server*/
int port;


Response readResponse(SSL* ssl) {
    int sizeRes;
    Response resp;
    char serialized[10000];

    if (SSL_read(ssl, &sizeRes, sizeof sizeRes) < 0) {
        resp.setMessage("Eroare la read() de la server.\n");
        resp.setCode(201);
    }

    if (SSL_read(ssl, serialized, sizeRes) < 0) {
        resp.setMessage("Eroare la read() de la server.\n");
        resp.setCode(202);
    }
    serialized[sizeRes] = 0;
    string serializedString = string(serialized);
    Response toReturn;
    toReturn.setCode(atoi(serializedString.substr(0, 3).c_str()));
    toReturn.setMessage(serializedString.substr(3));

    return toReturn;
}


int main(int argc, char *argv[]) {

    SSL_library_init();
    SSL_load_error_strings();


    const SSL_METHOD *method = SSLv3_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if(ctx == nullptr)
    {
        cerr << "Error creating context";
        return -1;
    }

    SSL* ssl = SSL_new(ctx);

    if(ssl == nullptr)
    {
        cerr << "Error creating SSL structure";
        SSL_CTX_free(ctx);
        return -2;
    }



    int sd;                    // descriptorul de socket
    struct sockaddr_in server; // structura folosita pentru conectare
    char msg[100];             // mesajul trimis

    /* exista toate argumentele in linia de comanda? */
    if (argc != 3) {
        printf("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    /* stabilim portul */
    port = atoi(argv[2]);

    /* cream socketul */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[client] Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons(port);

    /* ne conectam la server */
    if (connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    SSL_set_fd(ssl,sd);
    if(SSL_connect(ssl) != 1)
    {
        ERR_print_errors_fp(stderr);
        cerr << "Error connecting to secured server";
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sd);
        return -3;
    }
    //SSL_set_connect_state(ssl);

    bool loggedIn = false, admin = false;

    while (true) {


        while (!loggedIn) {
            bzero(msg, 100);
            printf("Nume: ");
            fflush(stdout);
            read(0, msg, 100);

            Request r = Request(msg);
            if (r.getRequest() == "Admin\n")
                admin = true;
            else
                admin = false;

            if (r.send(ssl) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }

            if (r.getRequest() == "exit\n" || r.getRequest() == "Exit\n")
                break;

            Response res = readResponse(ssl);

            if (res.getCode() == 103) // name is correct
            {
                bzero(msg, 100);
                printf("Password: ");
                fflush(stdout);
                read(0, msg, 100);
                if (msg[strlen(msg) - 1] == '\n')
                    msg[strlen(msg) - 1] = '\0';

                string hashedPassword = *new string(Crypto::sha256(msg));
                Request r2 = Request(hashedPassword.c_str());

                if (r2.send(ssl) <= 0) {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }

                if (r2.getRequest() == "exit\n" || r2.getRequest() == "Exit\n")
                    break;
            } else if (res.getCode() == 202) {
                printf("This account does not exist.\n");
            }
            res = readResponse(ssl);
            if (res.getCode() == 101) {
                printf("Succes.\n");
                loggedIn = true;
            }
            if (res.getCode() == 201) {
                printf("LogIn failed.\n");
            }
        }

        if(!loggedIn)
            return 0;


        bzero(msg, 100);
        printf("\nIntroduceti comanda: ");
        fflush(stdout);
        read(0, msg, 100);

        Request r = Request(msg);

        cout << r.getRequest();
        cout.flush();


        if (r.send(ssl) <= 0) {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }

        if (r.getRequest() == "exit\n" || r.getRequest() == "Exit\n")
            break;
        if ((r.getRequest() == "create\n" || r.getRequest() == "Create\n") && admin)
        {
            Response res = readResponse(ssl);
            while (res.getCode() != 101 && res.getCode() != 204) {
                cout << res.getMessage();
                cout.flush();
                bzero(msg, 100);
                read(0, msg, 100);
                Request req = Request(msg);
                req.send(ssl);
                res = readResponse(ssl);
            }
            cout << res.getMessage();
            cout.flush();
            continue;
        }

        Response res = readResponse(ssl);
        while (res.getCode() != 101) {
            cout << "[client]Mesajul primit este: \n" << res.getMessage() << endl;
            res = readResponse(ssl);
        }
    }
    /* inchidem conexiunea, am terminat */
    close(sd);
}

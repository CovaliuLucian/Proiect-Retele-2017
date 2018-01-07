/* cliTCP.c - Exemplu de client TCP 
   Trimite un nume la server; primeste de la server "Hello nume".
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009

*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#include "request.h"
#include "response.h"

#include <iostream>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <string>
#include <iomanip>
#include <sstream>

using namespace std;

/* portul de conectare la server*/
int port;

Response readResponse(int sd) {
    int sizeRes;
    Response resp;
    char serialized[10000];

    if (read(sd, &sizeRes, sizeof sizeRes) < 0) {
        resp.setMessage("Eroare la read() de la server.\n");
        resp.setCode(201);
    }

    if (read(sd, serialized, sizeRes) < 0) {
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

string sha256(const string &str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for (unsigned char i : hash) {
        ss << hex << setw(2) << setfill('0') << (int) i;
    }
    return ss.str();
}

int main(int argc, char *argv[]) {

    // SSL_library_init();
    // SSL_load_error_strings();




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

    bool loggedIn = false;

    while (true) {


        while (!loggedIn) {
            bzero(msg, 100);
            printf("Nume: ");
            fflush(stdout);
            read(0, msg, 100);

            Request r = Request(msg);

            if (r.send(sd) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }

            if (r.getRequest() == "exit\n" || r.getRequest() == "Exit\n")
                break;

            Response res = readResponse(sd);

            if (res.getCode() == 103) // name is correct
            {
                bzero(msg, 100);
                printf("Password: ");
                fflush(stdout);
                read(0, msg, 100);

                string hashedPassword = *new string(sha256(msg));
                Request r2 = Request(hashedPassword.c_str());

                if (r2.send(sd) <= 0) {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }

                if (r2.getRequest() == "exit\n" || r2.getRequest() == "Exit\n")
                    break;
            } else if (res.getCode() == 202) {
                printf("This account does not exist.\n");
            }
            res = readResponse(sd);
            if (res.getCode() == 101) {
                printf("Succes.\n");
                loggedIn = true;
            }
            if (res.getCode() == 201) {
                printf("LogIn failed.\n");
            }
        }


        bzero(msg, 100);
        printf("[client]Introduceti comanda: ");
        fflush(stdout);
        read(0, msg, 100);

        Request r = Request(msg);

        cout << r.getRequest();
        cout.flush();


        if (r.send(sd) <= 0) {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }

        if (r.getRequest() == "exit\n" || r.getRequest() == "Exit\n")
            break;


        Response res = readResponse(sd);
        while (res.getCode() != 101) {
            cout << "[client]Mesajul primit este: \n" << res.getMessage() << endl;
            res = readResponse(sd);
        }
    }
    /* inchidem conexiunea, am terminat */
    close(sd);
}

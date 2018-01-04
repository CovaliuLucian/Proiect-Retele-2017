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
#include <string>

using namespace std;

/* portul de conectare la server*/
int port;

Response readResponse(int sd)
{
  int sizeRes;
  Response resp;
  char serialized[10000];

  if (read(sd, &sizeRes, sizeof sizeRes) < 0)
  {
    resp.setMessage("Eroare la read() de la server.\n");
    resp.setCode(201);
  }

  if (read(sd, serialized, sizeRes) < 0)
  {
    resp.setMessage("Eroare la read() de la server.\n");
    resp.setCode(202);
  }
  serialized[sizeRes] = 0;
  string serializedString = string(serialized);
  Response toReturn;
  toReturn.setCode(atoi(serializedString.substr(0,3).c_str()));
  toReturn.setMessage(serializedString.substr(3));
  
  return toReturn;
}

int main(int argc, char *argv[])
{
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare
  char msg[100];             // mesajul trimis

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
  {
    printf("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  /* stabilim portul */
  port = atoi(argv[2]);

  /* cream socketul */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
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
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }

    while(true) {
        /* citirea mesajului */
        bzero(msg, 100);
        printf("[client]Introduceti comanda: ");
        fflush(stdout);
        read(0, msg, 100);

        Request r = Request(msg);

        cout << r.getRequest();
        cout.flush();

        /* trimiterea mesajului la server */
        if (r.send(sd) <= 0) {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }

        if(r.getRequest() == "exit\n" || r.getRequest() == "Exit\n")
            break;

        /* citirea raspunsului dat de server
           (apel blocant pina cind serverul raspunde) */

        Response res = readResponse(sd);
        while(res.getCode() != 101)
        {
            cout << "[client]Mesajul primit este: \n" << res.getMessage() << endl;
            res = readResponse(sd);
        }
    }
  /* inchidem conexiunea, am terminat */
  close(sd);
}

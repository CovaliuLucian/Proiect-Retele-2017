/* servTCPCSel.c - Exemplu de server TCP concurent 
   
   Asteapta un "nume" de la clienti multipli si intoarce clientilor sirul
   "Hello nume" corespunzator; multiplexarea intrarilor se realizeaza cu select().
   
   Cod sursa preluat din [Retele de Calculatoare,S.Buraga & G.Ciobanu, 2003] si modificat de 
   Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
   
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#include "response.h"
#include "request.h"
#include "parsing.h"
#include "token.h"
#include "tree.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>

#include <iostream>
#include <queue>
#include <string>

using namespace std;

/* portul folosit */

#define PORT 2728

int sayHello(int fd);

Request readRequest(int sd)
{
  char serialized[200];
  int length;
  int status = read(sd, &length, sizeof length);
  if (status < 0)
  {
    //error
  }
  int bytes = read(sd, serialized, length);
  if (length != bytes || bytes < 0)
  {
    perror("Eroare la read() de la client.\n");
    return 0;
  }
  serialized[length] = 0;
  if (serialized[length - 1] == '\n')
    serialized[length - 1] = 0;
  return Request(serialized);
}

extern int errno; /* eroarea returnata de unele apeluri */

/* functie de convertire a adresei IP a clientului in sir de caractere */
char *conv_addr(struct sockaddr_in address)
{
  static char str[25];
  char port[7];

  /* adresa IP a clientului */
  strcpy(str, inet_ntoa(address.sin_addr));
  /* portul utilizat de client */
  bzero(port, 7);
  sprintf(port, ":%d", ntohs(address.sin_port));
  strcat(str, port);
  return (str);
}

/* programul */
int main()
{
  struct sockaddr_in server; /* structurile pentru server si clienti */
  struct sockaddr_in from;
  fd_set readfds;    /* multimea descriptorilor de citire */
  fd_set actfds;     /* multimea descriptorilor activi */
  struct timeval tv; /* structura de timp pentru select() */
  int sd, client;    /* descriptori de socket */
  int optval = 1;    /* optiune folosita pentru setsockopt()*/
  int fd;            /* descriptor folosit pentru 
				   parcurgerea listelor de descriptori */
  int nfds;          /* numarul maxim de descriptori */
  unsigned int len;  /* lungimea structurii sockaddr_in */

  /* creare socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server] Eroare la socket().\n");
    return errno;
  }

  /*setam pentru socket optiunea SO_REUSEADDR */
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  /* pregatim structurile de date */
  bzero(&server, sizeof(server));

  /* umplem structura folosita de server */
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(PORT);

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server] Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen(sd, 5) == -1)
  {
    perror("[server] Eroare la listen().\n");
    return errno;
  }

  /* completam multimea de descriptori de citire */
  FD_ZERO(&actfds);    /* initial, multimea este vida */
  FD_SET(sd, &actfds); /* includem in multime socketul creat */

  tv.tv_sec = 1; /* se va astepta un timp de 1 sec. */
  tv.tv_usec = 0;

  /* valoarea maxima a descriptorilor folositi */
  nfds = sd;

  printf("[server] Asteptam la portul %d...\n", PORT);
  fflush(stdout);

  /* servim in mod concurent clientii... */
  while (1)
  {
    /* ajustam multimea descriptorilor activi (efectiv utilizati) */
    bcopy((char *)&actfds, (char *)&readfds, sizeof(readfds));

    /* apelul select() */
    if (select(nfds + 1, &readfds, NULL, NULL, &tv) < 0)
    {
      perror("[server] Eroare la select().\n");
      return errno;
    }
    /* vedem daca e pregatit socketul pentru a-i accepta pe clienti */
    if (FD_ISSET(sd, &readfds))
    {
      /* pregatirea structurii client */
      len = sizeof(from);
      bzero(&from, sizeof(from));

      /* a venit un client, acceptam conexiunea */
      client = accept(sd, (struct sockaddr *)&from, &len);

      /* eroare la acceptarea conexiunii de la un client */
      if (client < 0)
      {
        perror("[server] Eroare la accept().\n");
        continue;
      }

      if (nfds < client) /* ajusteaza valoarea maximului */
        nfds = client;

      /* includem in lista de descriptori activi si acest socket */
      FD_SET(client, &actfds);

      printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n", client, conv_addr(from));
      fflush(stdout);
    }
    /* vedem daca e pregatit vreun socket client pentru a trimite raspunsul */
    for (fd = 0; fd <= nfds; fd++) /* parcurgem multimea de descriptori */
    {
      /* este un socket de citire pregatit? */
      if (fd != sd && FD_ISSET(fd, &readfds))
      {
        if (sayHello(fd))
        {
          printf("[server] S-a deconectat clientul cu descriptorul %d.\n", fd);
          fflush(stdout);
          close(fd);           /* inchidem conexiunea cu clientul */
          FD_CLR(fd, &actfds); /* scoatem si din multime */
        }
      }
    } /* for */
  }   /* while */
} /* main */

/* realizeaza primirea si retrimiterea unui mesaj unui client */
int sayHello(int fd)
{
  char buffer[100]; /* mesajul */
  Request r;        //mesajul primit de la client
  Response res;     //mesaj de raspuns pentru client

  r = readRequest(fd);

  cout << "[server]Mesajul a fost receptionat..." << r.getRequest() << endl;

  queue<Token> test = Parser::ParsePrep(r.getRequest());

  queue<Token> test2 = Parser::SYA(test);

  while (!test.empty())
  {
    cout << test.front().command << " " << test.front().getType() << " " << test.front().priority << "\n";
    test.pop();
  }

  // while (!test2.empty())
  // {
  //   cout << test2.front().command << "\n";
  //   test2.pop();
  // }

  Tree test3 = Parser::GenerateTree(test2);

  cout << Parser::Execute(test3);

  
  // FILE *fp;
  // char path[1000];
  // char msg[1000];
  // bzero(msg, 1000);
  // fp = popen(r.getRequest().c_str(), "r");
  // while (fgets(path, sizeof(path), fp) != NULL)
  // {
  //   strcat(msg, path);
  // }

  // pclose(fp);

  // res.setMessage(string(msg));
  

  res.setMessage("pls work");
  res.setCode(100);

  cout << "[server]Trimitem mesajul inapoi..." << res.getMessage() << endl;

  res.send(fd);

  return 1;
}

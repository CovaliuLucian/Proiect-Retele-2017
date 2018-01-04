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
#include <cerrno>
#include <cstdio>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>
#include <pthread.h>

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


using namespace std;

/* portul folosit */

#define PORT 2728

Request readRequest(int sd) {
    char serialized[200];
    int length;
    long status = read(sd, &length, sizeof length);
    if (status < 0) {
        perror("Eroare la read() de la client.\n");
        Request err;
        err.setStatus(false);
        return err;
    }
    long bytes = read(sd, serialized, length);
    if (length != bytes || bytes < 0) {
        perror("Eroare la read() de la client.\n");
        Request err;
        err.setStatus(false);
        return err;
    }
    serialized[length] = 0;
    if (serialized[length - 1] == '\n')
        serialized[length - 1] = 0;
    return Request(serialized);
}


typedef struct thData {
    int idThread; //id-ul thread-ului tinut in evidenta de acest program
    int cl; //descriptorul intors de accept
} thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *arg);

int main() {
    struct sockaddr_in server;    // structura folosita de server
    struct sockaddr_in from;
    int nr;        //mesajul primit de trimis la client
    int sd;        //descriptorul de socket
    int pid;
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    int i = 0;


    /* crearea unui socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[server]Eroare la socket().\n");
        return errno;
    }
    /* utilizarea optiunii SO_REUSEADDR */
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    /* pregatirea structurilor de date */
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons(PORT);

    /* atasam socketul */
    if (bind(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen(sd, 2) == -1) {
        perror("[server]Eroare la listen().\n");
        return errno;
    }
    /* servim in mod concurent clientii...folosind thread-uri */
    bool run = true;
    while (run) {
        int client;
        thData *td; //parametru functia executata de thread
        socklen_t length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        //client= malloc(sizeof(int));
        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ((client = accept(sd, (struct sockaddr *) &from, &length)) < 0) {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        /* s-a realizat conexiunea, se astepta mesajul */

        int idThread; //id-ul threadului
        int cl; //descriptorul intors de accept

        td = (struct thData *) malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;

        pthread_create(&th[i], NULL, &treat, td);
    }//while
};

static void *treat(void *arg) {
    struct thData tdL;
    tdL = *((struct thData *) arg);
    printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush(stdout);
    pthread_detach(pthread_self());
    raspunde((struct thData *) arg);
    /* am terminat cu acest client, inchidem conexiunea */
    close((intptr_t) arg);
    return (NULL);

};

void raspunde(void *arg) {
    int nr, i = 0;
    struct thData tdL;
    tdL = *((struct thData *) arg);
    int fd = tdL.cl;
    Request r;        //mesajul primit de la client
    Response res;     //mesaj de raspuns pentru client

    while (true) {
        r = readRequest(fd);
        if(!r.getStatus())
            break;

        cout << "[server]Mesajul a fost receptionat..." << r.getRequest() << endl;

        if (r.getRequest() == "exit" || r.getRequest() == "Exit") {
            break;
        }

        try {
            Parser::Parse(r.getRequest(), fd);//STDIN_FILENO);
        }
        catch (string &m) {
            cout << "Error: " << m << "\n";
        }


        res.setMessage("Done");
        res.setCode(101);

        cout << "[server]Trimitem mesajul inapoi..." << res.getMessage() << endl;

        res.send(fd);
    }

    //return;
}

#include <sys/types.h>
#include <sys/socket.h>
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
#include "DataBase.h"
#include "Crypto.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>


#include <iostream>


using namespace std;

DataBase db;
SSL_CTX *ctx;

/* portul folosit */

#define PORT 2728


Request readRequest(SSL* ssl) {
    char serialized[200];
    int length;
    long status = SSL_read(ssl, &length, sizeof length);
    if (status < 0) {
        perror("Eroare la read() de la client.\n");
        ERR_print_errors_fp(stderr);
        Request err;
        err.setStatus(false);
        return err;
    }
    long bytes = SSL_read(ssl, serialized, length);
    if (length != bytes || bytes < 0) {
        perror("Eroare la read() de la client.\n");
        ERR_print_errors_fp(stderr);
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
    SSL* ssl;
} thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *arg);
bool Security()
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();

    const SSL_METHOD *method = SSLv3_method();
    ctx = SSL_CTX_new(method);
    if(ctx == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    if(SSL_CTX_use_certificate_file(ctx,"../cert.pem",SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    if(SSL_CTX_use_PrivateKey_file(ctx,"../cert.pem",SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    if(!SSL_CTX_check_private_key(ctx))
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    return true;
}

bool getSSL(SSL*& ssl)
{
    ssl = SSL_new(ctx);
    if(ssl == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    return true;
}

int main() {
    struct sockaddr_in server;    // structura folosita de server
    struct sockaddr_in from;
    int nr;        //mesajul primit de trimis la client
    int sd;        //descriptorul de socket
    int pid;
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    int i = 0;

    if (!db.Prepare("SSH")) return 0;

    SSL* ssl;

    if (!Security()) return 0;


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
        if(getSSL(ssl) < 0)
        {
            ERR_print_errors_fp(stderr);
            cerr << "Error creating SSL struct";
            continue;
        }
        int client;
        thData *td; //parametru functia executata de thread
        socklen_t length = sizeof(from);

        printf("\n[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        //client= malloc(sizeof(int));
        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ((client = accept(sd, (struct sockaddr *) &from, &length)) < 0) {
            perror("[server]Eroare la accept().\n");
            continue;
        }
        SSL_set_fd(ssl, client);
        if(SSL_accept(ssl)!=1)
        {
            ERR_print_errors_fp(stderr);
            cerr << "Error accepting secured connection: ";
            continue;
        }
        //SSL_set_accept_state(ssl);

        /* s-a realizat conexiunea, se astepta mesajul */

        int idThread; //id-ul threadului
        int cl; //descriptorul intors de accept

        td = (struct thData *) malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;
        td->ssl = ssl;

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
    bool loggedIn = false, admin = false;
    SSL* ssl = tdL.ssl;

    while (!loggedIn) {
        r = readRequest(ssl);
        if (!r.getStatus())
            break;

        cout << "[server]Mesajul a fost receptionat..." << r.getRequest() << endl;

        if (r.getRequest() == "exit" || r.getRequest() == "Exit") {
            break;
        }


        if (db.CheckAccount(r.getRequest())) {
            string name = r.getRequest();

            res.setMessage("Account iz good");
            res.setCode(103);

            if(res.send(ssl) <= 0)
                break;

            r = readRequest(ssl);
            if (!r.getStatus())
                break;

            cout << "[server]Mesajul a fost receptionat..." << r.getRequest() << endl;

            if (r.getRequest() == "exit" || r.getRequest() == "Exit") {
                break;
            }

            if (db.CheckAccount(name, r.getRequest())) {
                loggedIn = true;
                if (name == "Admin")
                    admin = true;
            }
        } else {
            res.setMessage("Account name not found");
            res.setCode(202);
            if(res.send(ssl) < 0)
                break;
        }

        res.setMessage("Done");
        if (loggedIn)
            res.setCode(101);
        else {
            res.setMessage("Not logged in");
            res.setCode(201);
        }

        cout << "[server]Trimitem mesajul inapoi..." << res.getMessage() << endl;

        if (res.send(ssl) == 0)
            break;
    }

    if (!loggedIn)
        return;

    while (true) {

        r = readRequest(ssl);
        if (!r.getStatus())
            break;

        cout << "[server]Mesajul a fost receptionat..." << r.getRequest() << endl;

        if (r.getRequest() == "exit" || r.getRequest() == "Exit") {
            break;
        } else if ((r.getRequest() == "create" || r.getRequest() == "Create") && admin) {

            res.setMessage("Name: ");
            res.setCode(100);
            res.send(ssl);
            r=readRequest(ssl);
            while(db.CheckAccount(r.getRequest()) || r.getRequest().empty())
            {
                res.setMessage("Name invalid or taken, try again: ");
                res.setCode(100);
                if(res.send(ssl) <= 0)
                    break;
                r=readRequest(ssl);
            }
            string name = r.getRequest();

            res.setMessage("Password: ");
            res.setCode(100);
            if(res.send(ssl) <= 0)
                break;
            r=readRequest(ssl);
            if(!r.getStatus())
                break;
            while(r.getRequest().empty())
            {
                res.setMessage("Not empty please, try again: ");
                res.setCode(100);
                if(res.send(ssl) <= 0)
                    break;
                r=readRequest(ssl);
                if (!r.getStatus())
                    break;
            }

            string password = r.getRequest();
            if (!r.getStatus() || r.getRequest() == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")
                break; // if hash of ""
            if (db.AddAccount(name,password,false))
            {
                res.setMessage("Account added.\n");
                res.setCode(101);
            } else
            {
                res.setMessage("Error adding account.");
                res.setCode(204);
            }
            if(res.send(ssl) <= 0)
                break;
            continue;


        } else if (r.getRequest() == "help" || r.getRequest() == "Help") {
            string toSend = string("");
            if (admin)
                toSend += "Create to create a new account.\n";
            toSend += "Help to get this message.\n";
            toSend += "Exit to exit the client.\n";
            res.setMessage(toSend);
            res.setCode(100);
            if(res.send(ssl) <= 0)
                break;
        } else
        {
            try {
                Parser::Parse(r.getRequest(), ssl);//STDIN_FILENO);
            }
            catch (string &m) {
                cout << "Error: " << m << "\n";
            }
        }


        res.setMessage("Done");
        res.setCode(101);

        cout << "[server]Trimitem mesajul inapoi..." << res.getMessage() << endl;

        res.send(ssl);
    }

    //return;
}

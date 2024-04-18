/***********************************************
*
* @Proposit: Implmenta les funcions de la llibreria Sockets.h que permeten la connexio entre clients i servidors.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 20/10/2023
* @Data ultima modificacio: 31/12/2023
*
************************************************/
#define _GNU_SOURCE

#include "Config.h"
#include "Sockets.h"

/***********************************************
*
* @Finalitat: Crea un servidor i retorna el file descriptor del socket. En el cas de que hi hagi algun error retorna -1.
* @Parametres: in: port_input, el port del servidor.
* @Retorn: int: el file descriptor del socket.
*
************************************************/
int createServer(char *port_input){
    uint16_t port;
    
    int aux = atoi (port_input);
    if (aux < 1 || aux > 65535){
        return (-1);
    }
    port = aux;

    int sockfd;
    sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        return (-1);
    }

    struct sockaddr_in s_addr;
    bzero (&s_addr, sizeof (s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons (port);
    s_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind (sockfd, (void *) &s_addr, sizeof (s_addr)) < 0){
        return (-1);
    }

    listen (sockfd, 5);
    return sockfd;
} 

/***********************************************
*
* @Finalitat: Accepta un client i retorna el file descriptor del socket. En el cas de que hi hagi algun error retorna -1.
* @Parametres: in: sockfd, el file descriptor del socket.
* @Retorn: int: el file descriptor del socket acceptat.
*
************************************************/
int acceptClient(int sockfd){
    struct sockaddr_in c_addr;
    socklen_t c_len = sizeof (c_addr);

    int newsock = accept (sockfd, (void *) &c_addr, &c_len);
    if (newsock < 0){
        return (-1);
    }
    return newsock;
}

/***********************************************
*
* @Finalitat: Es connecta a un servidor i retorna el file descriptor del socket. En el cas de que hi hagi algun error retorna -1.
* @Parametres: in: ip_name, la ip del servidor.
*              in: port_name, el port del servidor.
* @Retorn: int: el file descriptor del socket.
************************************************/
int connectToServer(char* ip_name, char* port_name){

    uint16_t port;
    int aux = atoi (port_name);
    if (aux < 1 || aux > 65535){
        return -1;
    }
    port = aux;

    struct in_addr ip_addr;
    if (inet_aton (ip_name, &ip_addr) == 0)
    {
        return -1;
    }

    int sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if ((sockfd) < 0){
        return -1;
    }

    struct sockaddr_in s_addr;
    bzero (&s_addr, sizeof (s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons (port);
    s_addr.sin_addr = ip_addr;

    if (connect ((sockfd), (void *) &s_addr, sizeof (s_addr)) < 0){
        return -1;
    }

    return sockfd;
}


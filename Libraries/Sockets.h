/***********************************************
*
* @Proposit: Llibreria que conté les funcions necessaries per a la connexio entre clients i servidors.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 20/10/2023
* @Data ultima modificacio: 31/12/2023
*
************************************************/
#ifndef SOCKETS_H
#define SOCKETS_H

/***********************************************
*
* @Finalitat: Crea un servidor i retorna el file descriptor del socket. En el cas de que hi hagi algun error retorna -1.
* @Parametres: in: port_input, el port del servidor.
* @Retorn: int: el file descriptor del socket.
*
************************************************/
int createServer(char *port_input);

/***********************************************
*
* @Finalitat: Accepta un client i retorna el file descriptor del socket. En el cas de que hi hagi algun error retorna -1.
* @Parametres: in: sockfd, el file descriptor del socket.
* @Retorn: int: el file descriptor del socket acceptat.
*
************************************************/
int acceptClient(int sockfd);

/***********************************************
*
* @Finalitat: Es connecta a un servidor i retorna el file descriptor del socket. En el cas de que hi hagi algun error retorna -1.
* @Parametres: in: ip_name, la ip del servidor.
*              in: port_name, el port del servidor.
* @Retorn: int: el file descriptor del socket.
************************************************/
int connectToServer(char* ip_name, char* port_name);

#endif
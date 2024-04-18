/***********************************************
*
* @Proposit: Llibreria que conté les funcions que permet al usuari interactuar amb el servidor. També conté les estructures de dades necessaries per a la seva execució
             i constants que s'utilitzen en el programa.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 13/10/2023
* @Data ultima modificacio: 8/1/2024
*
************************************************/
#ifndef SOCKET_H_
#define SOCKET_H_

#include "Config.h"
#include "TextUtilLib.h"
#include "Comunication.h"
#include "Sockets.h"

#define ERROR 1
#define SUCCESS 0

#define ERROR_FILE "Error al abrir el archivo\n\n"
#define ERROR_COMMAND "ERROR: Please input a valid command.\n\n"
#define ERROR_DOWNLOAD "Cannot download, you are not connected to HAL 9000.\n\n $ "
#define ERROR_ARGUMENTS "ERROR: Please input a valid number of arguments.\n\n"
#define ERROR_USERNAME "ERROR: Username cannot contain '&'.\n\n"
#define ERROR_CONNECT "ERROR: Cannot connect to HAL 9000 first.\n\n"
#define ERROR_LOGOUT "ERROR: Could not logout from server. Forced exit.\n\n"
#define NO_DOWNLOADS "There are no downloads\n\n"
#define POOLE_DISCONNECTED "\rPoole disconnected. Please reconect to a new Server.\n\n$ "
#define DOWNLOAD_NOT_FOUND "\rERROR: Download not found.\n\n$ "
#define DOWNLOAD_FAILED " was downloaded incorrectly.\n\n$ "

#define DOWNLOAD "DOWNLOAD"
#define CONNECT "CONNECT"
#define LOGOUT "LOGOUT"
#define LIST "LIST"
#define SONGS "SONGS"
#define PLAYLISTS "PLAYLISTS"
#define CHECK "CHECK"
#define DOWNLOADS "DOWNLOADS"
#define CLEAR "CLEAR"

#define SUCCESS_CONNECT " connected to HAL 9000 system, welcome music lover!\n\n"
#define SUCCESS_LOGOUT "Thanks for using HAL 9000, see you soon, music lover!\n\n"
#define ERROR_SONGS "\rERROR: Could not get songs from server.\n\n$ "
#define ERROR_PLAYLISTS "\rERROR: Could not get playlists from server.\n\n$ "
#define DOWNLOAD_START "\rDownload started\n\n$ "
#define SUCCESS_DOWNLOAD " was downloaded successfully\n\n$ "

typedef struct{
    char *user_name;
    char *folder_name;
    char *ip_discovery_user;
    char *port_discovery_user;
    char connected;
    int poole_sockfd;
    int num_downloads;
    int pipe_monolit;
    int reconnected;
}User;

typedef struct{
    char *id;
    int total_bytes;
    int current_bytes;
    char *file_name;
    char *md5sum;
    int fd_file;
    int completed;
    int initialized;
    char *playlist_name;
}Download;

/***********************************************
*
* @Finalitat: Desconnecta un usuari del servidor.
* @Parametres: in: user = l'usuari que es desconnecta.
*              in: header = el header del missatge.
*              in: sockfd_poole = el socket del poole.
*              in: downloads = la llista de descarregues.
* @Retorn: void
*
************************************************/
void disconnectUser(User *user, char *header, int sockfd_poole, LinkedList *downloads);

/***********************************************
*
* @Finalitat: Connecta un usuari al servidor.
* @Parametres: in: user = l'usuari que es connecta.
*              in: sockfd_poole = el socket del poole.
* @Retorn: int: 1 si s'ha connectat correctament, 0 altrament.
*
************************************************/
int connectUser(User *user, int *sockfd_poole);

/***********************************************
*
* @Finalitat: Envia un missatge per el socket donat.
* @Parametres: in: sockfd = el socket pel qual s'envia el missatge.
*              in: header = el header del missatge.
*              in: data = les dades del missatge.
*              in: type = el tipus de missatge.
* @Retorn: int: 0 si s'ha enviat correctament, -1 altrament.
*
************************************************/
int sendMessage(int sockfd,char *header, char *data, char type);

#endif
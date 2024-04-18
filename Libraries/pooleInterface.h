/***********************************************
*
* @Proposit: Llibreria que conté les funcions que necessiten els servidors poole per a funcionar correctament.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 20/10/2023
* @Data ultima modificacio: 8/1/2024
*
************************************************/
#ifndef POOLEINTERFACE_H
#define POOLEINTERFACE_H

#define SONGS_FOLDER "songs"
#define PLAYLISTS_FOLDER "playlists"
#define FILE_ERROR "Error al abrir el archivo\n"
#define CONNECTING "Connecting Smyslov Server to the system..\n"
#define CONNECTED "Connected to HAL 9000 System, ready to listen to Bowmans petitions\n\nWaiting for connections...\n"

typedef struct{
    char* name;
    char** songs;
    int songs_count;
}Playlist;

typedef struct{
    char *name;
    char *folder_name;
    char *port_discovery;
    char *ip_discovery;
    char *port_poole;
    char *ip_poole;
    int usersConnected;
}Poole;

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

/***********************************************
*
* @Finalitat: Envia un missatge de desconnexio al servidor.
* @Parametres: in: sockfd, el socket del servidor.
* @Retorn: void
*
************************************************/
void disconnect(int sockfd);

/***********************************************
*
* @Finalitat: Es connecta al servidor discovery i retorna el file descriptor del socket.
* @Parametres: in: ip_discovery, la ip del servidor discovery.
*              in: port_discovery, el port del servidor discovery.
* @Retorn: int: el file descriptor del socket.
*
************************************************/
int connectDiscovery(char *ip_discovery, char *port_discovery);

/***********************************************
*
* @Finalitat: Retorna una llista amb els noms de les cançons que hi ha al servidor.
* @Parametres: in: folderName, el nom del directori on es troben les cançons.
*              in/out: song_count, el nombre de cançons que hi ha al servidor.
* @Retorn: char **: la llista amb els noms de les cançons.
*
************************************************/
char **getSongList(char *folderName, int *song_count);

/***********************************************
*
* @Finalitat: Retorna una llista amb els noms de les playlists que hi ha al servidor.
* @Parametres: in: folderName, el nom del directori on es troben les playlists.
*              in/out: playlists_count, el nombre de playlists que hi ha al servidor.
*              in/out: songs_count, el nombre de cançons que hi ha a cada playlist.
* @Retorn: char ***: la llista amb els noms de les playlists.
*
************************************************/
char ***getPlaylists(char *folderName, int *playlists_count, int **songs_count);

/***********************************************
*
* @Finalitat: Comprova si una cançó existeix al servidor.
* @Parametres: in: songName, el nom de la cançó.
*              in: folderName, el nom del directori on es troba la cançó.
* @Retorn: char *: el nom de la cançó si existeix, NULL altrament.
*
************************************************/
char *checkExistingSong(char *songName, char *folderName);

/***********************************************
*
* @Finalitat: Retorna una llista amb els noms de les cançons que hi ha a una playlist.
* @Parametres: in: playlistName, el nom de la playlist.
*              in: folderName, el nom del directori on es troba la playlist.
*              in/out: song_count, el nombre de cançons que hi ha a la playlist.
* @Retorn: char **: la llista amb els noms de les cançons.
*
************************************************/
char **getSongListFromPlaylist(char *playlistName, char *folderName, int *song_count);


#endif
/***********************************************
*
* @Proposit: Codifica i decodifica els missatges que es reben i s'envien mitjançant el protocol de comunicació. Calcula el MD5 d'un fitxer.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 10/11/2023
* @Data ultima modificacio: 6/1/2024
*
************************************************/
#ifndef COMUNICATION_H
#define COMUNICATION_H

#define NEW_POOLE "NEW_POOLE"
#define CON_OK "CON_OK\0"
#define CON_KO "CON_KO\0"

#define NEW_BOWMAN "NEW_BOWMAN"
#define LIST_SONGS "LIST_SONGS"
#define SONGS_RESPONSE "SONGS_RESPONSE"
#define SONGS_NUMBER "SONGS_NUMBER"
#define LIST_PLAYLISTS "LIST_PLAYLISTS"
#define PLAYLISTS_RESPONSE "PLAYLISTS_RESPONSE"
#define PLAYLISTS_NUMBER "PLAYLISTS_NUMBER"
#define EXIT "EXIT"
#define UNKNOWN "UNKNOWN"
#define DOWNLOAD_SONG "DOWNLOAD_SONG"
#define DOWNLOAD_LIST "DOWNLOAD_LIST"
#define FILE_DATA "FILE_DATA"
#define NEW_FILE "NEW_FILE"
#define DATA_KO "DATA_KO"
#define DATA_OK "DATA_OK"
#define CHECK_OK "CHECK_OK"
#define CHECK_KO "CHECK_KO"
#define DISCONNECT "DISCONNECT"

#define SONGS_FOLDER "songs"
#define PLAYLISTS_FOLDER "playlists"

#define POOLE 1
#define BOWMAN 0

/***********************************************
*
* @Finalitat: Deodifica un missatge codificat amb el protocol de comunicació.
* @Parametres: in: buffer: buffer que conte el missatge codificat.
*              in/out: count: nombre de camps que conte el missatge.
               in: type: tipus de missatge que es vol decodificar.
* @Retorn: char**: array de strings que conte els camps del missatge.
*
************************************************/
char** decodeMessage(char *buffer, int *count, int type);

/***********************************************
*
* @Finalitat: Codifica un missatge amb el protocol de comunicació.
* @Parametres: in: header: header del missatge.
*              in: data: dades del missatge.
*              in/out: buffer: buffer on es codificara el missatge.
               in: type: tipus de missatge que es vol codificar.
* @Retorn: void
*
************************************************/
void encodeMessage(char* header, char* data, char* buffer, char type);

/***********************************************
*
* @Finalitat: Junta dos buffers en un de sol.
* @Parametres: in/out: finalBuffer: buffer on es guardara el resultat.
*              in: bufferToJoin: buffer que es vol afegir.
*              in: first: indica si es el primer buffer que s'afegeix.
               in: buffer_num: nombre de buffers que es volen afegir.
               in: header: header del missatge.
* @Retorn: void
*
************************************************/
void joinBuffers(char** finalBuffer, char* bufferToJoin, int first, int buffer_num, char* header);

/***********************************************
*
* @Finalitat: Calcula el MD5 d'un fitxer.
* @Parametres: in: filename: nom del fitxer.
*              in/out: md5: string on es guardara el resultat.
               in: folderName: nom del directori on es troba el fitxer.
               in: playlist_name: nom de la playlist on es troba el fitxer.
* @Retorn: int: 0 si tot ha anat be, 1 si hi ha hagut algun error.
*
************************************************/
int calculateMD5(const char *filename, char *md5, char *folderName, char *playlist_name);

#endif
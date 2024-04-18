/***********************************************
*
* @Proposit: Fitxer que implmenta les funcions de la interficie de poole. poolesInterface.h.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 20/10/2023
* @Data ultima modificacio: 8/1/2024
*
************************************************/
#define _GNU_SOURCE
#include "Config.h"
#include "Comunication.h"
#include <dirent.h>
#include "pooleInterface.h"
#include "Sockets.h"
#include "TextUtilLib.h"
#include <sys/wait.h>

/***********************************************
*
* @Finalitat: Envia un missatge de desconnexio al servidor.
* @Parametres: in: sockfd, el socket del servidor.
* @Retorn: void
*
************************************************/
void disconnect(int sockfd){
    char buffer[256];
    memset(buffer, '\0', 256);
    encodeMessage(CON_OK, NULL, buffer, 0x06);
    write(sockfd, buffer, 256);
}

/***********************************************
*
* @Finalitat: Es connecta al servidor discovery i retorna el file descriptor del socket.
* @Parametres: in: ip_discovery, la ip del servidor discovery.
*              in: port_discovery, el port del servidor discovery.
* @Retorn: int: el file descriptor del socket.
*
************************************************/
int connectDiscovery(char *ip_discovery, char *port_discovery){
    char buffer[256];
    memset(buffer, '\0', 256);
    int sockfd = connectToServer(ip_discovery, port_discovery);
    if(sockfd < 0){
        return -1;
    }
    int count;
    read(sockfd, buffer, 256);
    char **decodedMessage = decodeMessage(buffer, &count, POOLE);
    
    if(strcmp(decodedMessage[0], CON_OK) == 0){
        free(decodedMessage[0]);
        free(decodedMessage);
        return sockfd;
    }else{
        free(decodedMessage[0]);
        free(decodedMessage);
        return -1;
    }

}

/***********************************************
*
* @Finalitat: Retorna una llista amb els noms de les cançons que hi ha al servidor.
* @Parametres: in: folderName, el nom del directori on es troben les cançons.
*              in/out: song_count, el nombre de cançons que hi ha al servidor.
* @Retorn: char **: la llista amb els noms de les cançons.
*
************************************************/
char **getSongList(char *folderName, int *song_count){
    char *buffer;
    asprintf(&buffer, "%s/%s", folderName, SONGS_FOLDER);
    DIR *dir = opendir(buffer);
    free(buffer);
    char **songs = (char **) malloc(sizeof(char *));
    *song_count = 0;
    if (dir != NULL) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            songs[*song_count] = (char *) malloc(sizeof(char) * (strlen(entry->d_name) + 1));
            strcpy(songs[*song_count], entry->d_name);
            (*song_count)++;
            songs = (char **) realloc(songs, sizeof(char *) * (*song_count + 1));

        }
        closedir(dir);
        return songs;
    } else {
        perror("Error opening directory");
        free(songs);
        return NULL;
    }
    
}

/***********************************************
*
* @Finalitat: Retorna una llista amb els noms de les playlists que hi ha al servidor.
* @Parametres: in: folderName, el nom del directori on es troben les playlists.
*              in/out: playlists_count, el nombre de playlists que hi ha al servidor.
*              in/out: songs_count, el nombre de cançons que hi ha a cada playlist.
* @Retorn: char ***: la llista amb els noms de les playlists.
*
************************************************/
char ***getPlaylists(char *folderName, int *playlists_count, int **songs_count) {
    char *buffer;
    asprintf(&buffer, "%s/%s", folderName, PLAYLISTS_FOLDER);
    DIR *dir = opendir(buffer);
    free(buffer);

    if (dir == NULL) {
        perror("Error opening directory");
        return NULL;
    }

    char ***playlists = NULL;
    *playlists_count = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".DS_Store") == 0) {
            continue;
        }

        playlists = (char ***)realloc(playlists, (*playlists_count + 1) * sizeof(char **));
        playlists[*playlists_count] = (char **)malloc(sizeof(char *));
        playlists[*playlists_count][0] = (char *)malloc(strlen(entry->d_name) + 1);
        strcpy(playlists[*playlists_count][0], entry->d_name);

        asprintf(&buffer, "%s/%s/%s", folderName, PLAYLISTS_FOLDER, entry->d_name);
        DIR *dir2 = opendir(buffer);
        free(buffer);

        if (dir2 != NULL) {
            int aux_song_count = 0;

            struct dirent *entry2;
            while ((entry2 = readdir(dir2)) != NULL) {
                if (strcmp(entry2->d_name, ".") == 0 || strcmp(entry2->d_name, "..") == 0) {
                    continue;
                }
                playlists[*playlists_count] = (char **)realloc(playlists[*playlists_count], (aux_song_count + 2) * sizeof(char *));
                playlists[*playlists_count][(aux_song_count) + 1] = (char *)malloc(strlen(entry2->d_name) + 1);
                strcpy(playlists[*playlists_count][(aux_song_count) + 1], entry2->d_name);
                (aux_song_count)++;
            }
            (*songs_count) = (int *)realloc((*songs_count), (*playlists_count + 1) * sizeof(int));
            (*songs_count)[(*playlists_count)] = aux_song_count;
            aux_song_count = 0;

            closedir(dir2);
            (*playlists_count)++;
        } else {
            return NULL;
        }
    }

    closedir(dir);
    return playlists;
}

/***********************************************
*
* @Finalitat: Comprova si una cançó existeix al servidor.
* @Parametres: in: songName, el nom de la cançó.
*              in: folderName, el nom del directori on es troba la cançó.
* @Retorn: char *: el nom de la cançó si existeix, NULL altrament.
*
************************************************/
char* checkExistingSong(char *songName, char *folderName){
    int songs_count;
    int i;
    char **songs = getSongList(folderName, &songs_count);
    for(i = 0; i < songs_count; i++){
        char *song_to_return = malloc(sizeof(char) * (strlen(songs[i]) + 1));
        strcpy(song_to_return, songs[i]);
        toUpperCase(songs[i]);
        if(strcmp(songs[i], songName) == 0){
            for(int k = 0; k < songs_count; k++){
                free(songs[k]);
            }
            free(songs);
            return song_to_return;
        }
        free(song_to_return);
    }
    for(int j = 0; j < songs_count; j++){
        free(songs[j]);
    }
    free(songs);
    return NULL;
}

/***********************************************
*
* @Finalitat: Retorna una llista amb els noms de les cançons que hi ha a una playlist.
* @Parametres: in: playlistName, el nom de la playlist.
*              in: folderName, el nom del directori on es troba la playlist.
*              in/out: song_count, el nombre de cançons que hi ha a la playlist.
* @Retorn: char **: la llista amb els noms de les cançons.
*
************************************************/
char **getSongListFromPlaylist(char *playlistName, char *folderName, int *song_count){
    int playlists_count;
    int *song_count_all = NULL;
    char ***playlists = getPlaylists(folderName, &playlists_count, &song_count_all);
    int i;
    int j;
    *song_count = 0;
    for(i = 0; i < playlists_count; i++){
        char *playlist_to_return = malloc(sizeof(char) * (strlen(playlists[i][0]) + 1));
        strcpy(playlist_to_return, playlists[i][0]);
        toUpperCase(playlist_to_return);
        if(strcmp(playlist_to_return, playlistName) == 0){
            free(playlist_to_return);
            char **songs = (char **) malloc(sizeof(char *));
            *song_count = 0;
            for(j = 0; j < song_count_all[i]+1; j++){
                songs[*song_count] = (char *) malloc(sizeof(char) * (strlen(playlists[i][j]) + 1));
                strcpy(songs[*song_count], playlists[i][j]);
                (*song_count)++;
                songs = (char **) realloc(songs, sizeof(char *) * (*song_count + 1));
            }

            for(i = 0; i < playlists_count; i++){
                for(j = 0; j < song_count_all[i]; j++){
                    free(playlists[i][j+1]);
                }
                free(playlists[i][0]);
                free(playlists[i]);
            }
            free(playlists);
            free(song_count_all);

            return songs;
        }
        free(playlist_to_return);
    }
    for(i = 0; i < playlists_count; i++){
        for(j = 0; j < song_count_all[i]; j++){
            free(playlists[i][j+1]);
        }
        free(playlists[i][0]);
        free(playlists[i]);
    }
    free(playlists);
    free(song_count_all);
    return NULL;

}
/***********************************************
*
* @Proposit: Implementacio de les funcions de Comunication.h. Es tracta de les funcions que permeten la comunicacio entre el client i el servidor.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 10/11/2023
* @Data ultima modificacio: 6/1/2024
*
************************************************/
#define _GNU_SOURCE
#include "Config.h"
#include "TextUtilLib.h"
#include "Comunication.h"
#include <sys/wait.h>

/***********************************************
*
* @Finalitat: Deodifica un missatge codificat amb el protocol de comunicació.
* @Parametres: in: buffer: buffer que conte el missatge codificat.
*              in/out: count: nombre de camps que conte el missatge.
               in: type: tipus de missatge que es vol decodificar.
* @Retorn: char**: array de strings que conte els camps del missatge.
*
************************************************/
char** decodeMessage(char* buffer, int *count, int type){
    int header_length = buffer[1] * 256 + buffer[2];
    char header[header_length];
    for (int i = 0; i < header_length; ++i) {
        header[i] = buffer[i + 3];
    }
    header[header_length] = '\0';

    if(strcmp(header, UNKNOWN) == 0){
        *count = 0;
        return NULL;
    }
    if(strcmp(header, PLAYLISTS_RESPONSE) == 0){
        if(buffer[3+header_length] == '\0'){
            *count = 1;
            char** data = (char**) malloc(sizeof(char*));
            data[0] = (char*) malloc(sizeof(header)+1);
            strcpy(data[0], header);
            return data;
        }else{
            char** data = splitByChar(buffer + 3 + header_length, '#', count);
            data = (char**) realloc(data, sizeof(char*) * (*count + 1));
            data[*count] = (char*) malloc(sizeof(header)+1);
            strcpy(data[*count], header);
            return data;
        }
    }else if(strcmp(header, FILE_DATA) == 0){
        char** data = NULL;
        data = splitByCharByTimes(buffer + 3 + header_length, '&', count, 1);
        data = (char**) realloc(data, sizeof(char*) * (*count + 1));
        data[*count] = (char*) malloc(sizeof(header)+1);
        data[*count][0] = '\0';
        strcpy(data[*count], header);
        return data;
    }

    if(strcmp(header, CON_OK) == 0 || strcmp(header, CON_KO) == 0){
        char** data;
        if(type == POOLE || strcmp(header, CON_KO) == 0){
            data = (char**) malloc(sizeof(char*));
            data[0] = (char*) malloc(sizeof(header)+1);
            strcpy(data[0], header);
            *count = 1;
        }else{
            if(strcmp(header, CON_OK) == 0){
                data = splitByChar(buffer + 3 + header_length, '&', count);
            }
        }
        return data;
    }else{
        if(buffer[3+header_length] == '\0'){
            *count = 1;
            char** data = (char**) malloc(sizeof(char*));
            data[0] = (char*) malloc(sizeof(header)+1);
            strcpy(data[0], header);
            return data;
        }else{
            char** data = NULL;
            data = splitByChar(buffer + 3 + header_length, '&', count);
            data = (char**) realloc(data, sizeof(char*) * (*count + 1));
            data[*count] = (char*) malloc(sizeof(header)+1);
            data[*count][0] = '\0';
            strcpy(data[*count], header);
            return data;
        }
    }
    return NULL;
}

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
void encodeMessage(char* header, char* data, char* buffer, char type){
    int header_length = strlen(header);
    buffer[0] = type;
    buffer[1] = header_length / 256;
    buffer[2] = header_length % 256;
    for (int i = 0; i < header_length; ++i) {
        buffer[i + 3] = header[i];
    }
    if(data != NULL){
        int data_length = strlen(data);
        for (int i = 0; i < data_length; ++i) {
            buffer[i + 3 + header_length] = data[i];
        }
    }
}

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
void joinBuffers(char** finalBuffer, char* bufferToJoin, int first, int buffer_num, char* header) {
    int header_length = strlen(header);
    if (first == 0) {
        *finalBuffer = (char*)realloc(*finalBuffer, (sizeof(char)) * (3 + header_length + 1));
        (*finalBuffer)[0] = 0x01;
        (*finalBuffer)[1] = header_length / 256;
        (*finalBuffer)[2] = header_length % 256;
        for (int i = 0; i < header_length; ++i) {
            (*finalBuffer)[i+3] = header[i];
        }
        (*finalBuffer)[3 + header_length] = '\0';
    }

    int data_length;
    if (first < buffer_num - 1) {
        data_length = 256 - (3 + header_length);
    } else {
        data_length = strlen(bufferToJoin + 3 + header_length);
    }

    int finalBuffer_length = strlen(*finalBuffer + 3 + header_length) + 3 + header_length;
    *finalBuffer = (char*)realloc(*finalBuffer, sizeof(char) * (finalBuffer_length + data_length + 1));
    
    for (int i = 0; i < data_length; i++) {
        (*finalBuffer)[finalBuffer_length + i] = bufferToJoin[3 + header_length + i];
    }

    (*finalBuffer)[finalBuffer_length + data_length] = '\0';
}

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
int calculateMD5(const char *filename, char *md5, char *folderName, char *playlist_name){
    int pipe_fd[2];
    
    if (pipe(pipe_fd) == -1) {
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        
        return 1;
    } else if (pid == 0) {
        dup2(pipe_fd[1], STDOUT_FILENO);

        close(pipe_fd[0]);
        close(pipe_fd[1]);

        char *path;
        if(playlist_name != NULL){
            asprintf(&path, "%s/%s/%s/%s", folderName, PLAYLISTS_FOLDER, playlist_name, filename);
        }else{
            asprintf(&path, "%s/%s/%s", folderName, SONGS_FOLDER, filename);
        }
        
        execlp("md5sum", "md5sum", path, (char *)NULL);
        free(path);

        exit(EXIT_FAILURE);
    } else {
        close(pipe_fd[1]);

        ssize_t bytesRead = read(pipe_fd[0], md5, 32);

        close(pipe_fd[0]);

        wait(NULL);

        md5[bytesRead] = '\0';
    }
    return 0;
}

/***********************************************
*
* @Proposit: Implementa les funcions de la llibreria userInterface.h que conté les funcions que permet al usuari interactuar amb el servidor.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 13/10/2023
* @Data ultima modificacio: 28/12/2023
*
************************************************/
#define _GNU_SOURCE
#include "userInterface.h"

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
int sendMessage(int sockfd,char *header, char *data, char type){
    char buffer[256];
    memset(buffer, '\0', 256);
    encodeMessage(header, data, buffer, type);
    write(sockfd, buffer, 256);
    return SUCCESS;
}

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
void disconnectUser(User *user, char *header, int sockfd_poole, LinkedList *downloads){
    if(user->connected == 1 || user->reconnected == 1){
        user->connected = 0;
        if(user->num_downloads>0){
            UniversalLinkedList_goToHead(*downloads);
            while(UniversalLinkedList_isAtEnd(*downloads) == 0){
                printf("asdasda");
                Download *download = (Download *) UniversalLinkedList_get(*downloads);
                free(download->file_name);
                free(download->md5sum);
                free(download->id);
                if(download->playlist_name != NULL){
                    free(download->playlist_name);
                }
                free(download);
                UniversalLinkedList_next(*downloads);
            }
            UniversalLinkedList_destroy(&(*downloads));
        }
        sendMessage(sockfd_poole, header, user->user_name, 0x06);
    }
}

/***********************************************
*
* @Finalitat: Connecta un usuari al servidor.
* @Parametres: in: user = l'usuari que es connecta.
*              in: sockfd_poole = el socket del poole.
* @Retorn: int: 1 si s'ha connectat correctament, 0 altrament.
*
************************************************/
int connectUser(User *user, int *sockfd_poole){
    char buffer[256];
    int sockfd = connectToServer(user->ip_discovery_user, user->port_discovery_user);
    if(sockfd < 0){
        return 0;
    }

    sendMessage(sockfd, NEW_BOWMAN,user->user_name, 0x01);
    read(sockfd, buffer, 256);
    int count;
    char **decodedMessage = decodeMessage(buffer, &count, BOWMAN);
    if(strcmp(decodedMessage[0], CON_KO) != 0){
        user->connected=1;
        *sockfd_poole = connectToServer(decodedMessage[1], decodedMessage[2]);
        if(*sockfd_poole < 0){
            user->connected=0;
            for(int i = 0; i < count; i++){
                free(decodedMessage[i]);
            }
            free(decodedMessage);
            close(sockfd);
            return 0;
        }

    }else{
        user->connected=0;
        for(int i = 0; i < count; i++){
            free(decodedMessage[i]);
        }
        free(decodedMessage);
        close(sockfd);
        return 0;
    }
    
    for(int i = 0; i < count; i++){
        free(decodedMessage[i]);
    }
    free(decodedMessage);

    close(sockfd);
    return 1;
}



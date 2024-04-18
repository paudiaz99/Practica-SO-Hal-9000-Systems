/***********************************************
*
* @Proposit: Servidor Discovery. Gestiona les peticions dels servidors Poole i dels clients Bowman.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 10/11/2023
* @Data ultima modificacio: 8/1/2024
*
************************************************/
#define _GNU_SOURCE
#include "../Libraries/Config.h"
#include "../Libraries/Sockets.h"
#include "../Libraries/Comunication.h"
#include "../Libraries/TextUtilLib.h"
#include "../Libraries/UniversalLinkedList.h"
#include "../Libraries/semaphore_v2.h"

#define ERROR 1
#define SUCCESS 0

#define ERROR_ARGUMENTS "ERROR: Please input a valid number of arguments.\n"
#define ERROR_THREAD "ERROR: Thread can not be created"
#define ERROR_FILE "Error: opening file\n"

typedef struct{
    char* ipP;
    char* portP;
    char* ipB;
    char* portB;
}Discovery;

typedef struct{
    char *name;
    char *folder_name;
    char *port_discovery;
    char *ip_discovery;
    char *port_poole;
    char *ip_poole;
    int usersConnected;
}Poole;


Discovery discovery;
LinkedList pooles;
semaphore print_mutex;
pthread_t thread_pooles, thread_bowmans;

int fd_reader;

/***********************************************
*
* @Finalitat: Printar per pantalla fent servir un semafor.
* @Parametres: in: x: String a printar.
* @Retorn: void
*
************************************************/
void printF(char *x){
    SEM_wait(&print_mutex);
    write(1, x, strlen(x));
    SEM_signal(&print_mutex);
}

/***********************************************
*
* @Finalitat: Funcio que s'executa quan es rep un SIGINT.
* @Parametres: in: signum: Senyal que s'ha rebut.
* @Retorn: void
*
************************************************/
void sigintHandler(){
    SEM_destructor(&print_mutex);
    signal(SIGINT, SIG_DFL);
    free(discovery.ipP);
    free(discovery.portP);
    free(discovery.ipB);
    free(discovery.portB);

    UniversalLinkedList_goToHead(pooles);
    while(UniversalLinkedList_isAtEnd(pooles) == 0){
        Poole* poole = (Poole*) UniversalLinkedList_get(pooles);
        free(poole->ip_poole);
        free(poole->port_poole);
        free(poole->name);
        free(poole);
        UniversalLinkedList_remove(pooles);
        UniversalLinkedList_goToHead(pooles);
    }
    UniversalLinkedList_destroy(&pooles);
    pthread_cancel(thread_pooles);
    pthread_cancel(thread_bowmans);
    printF("\nDiscovery closed successfully\n");
    raise(SIGINT);
}

/***********************************************
*
* @Finalitat: Llegir el fitxer de configuracio.
* @Parametres: in: void
* @Retorn: void
*
************************************************/
void readFile(){
    readText(&discovery.ipP, fd_reader, '\n');
    readText(&discovery.portP, fd_reader, '\n');
    readText(&discovery.ipB, fd_reader, '\n');
    readText(&discovery.portB, fd_reader, '\n');
}

/***********************************************
*
* @Finalitat: Funció que s'executa en un thread. S'encarrega de gestionar les peticions dels servidors Poole.
* @Parametres: in: void
* @Retorn: void
*
************************************************/
void* socketP(){
    int sockfd = createServer(discovery.portP);
    char buffer[256];
    char *aux_buffer;

    while(1){
        int newsockfd = acceptClient(sockfd);
        memset(buffer, '\0', 256);
        if(newsockfd < 0){
            printF("Error: accept\n");
            exit(1);
        }
        encodeMessage(CON_OK, NULL, buffer, 0x01);
        write(newsockfd, buffer, 256);
        read(newsockfd, buffer, 256);
        int count = 0;
        char** data = decodeMessage(buffer, &count, POOLE);
        if(strcmp(data[count], NEW_POOLE) == 0){
            Poole aux_poole; 
            aux_poole.name = (char*) malloc(sizeof(char) * strlen(data[0])+1);
            aux_poole.ip_poole = (char*) malloc(sizeof(char) * strlen(data[1])+1);
            aux_poole.port_poole = (char*) malloc(sizeof(char) * strlen(data[2])+1);
            aux_poole.usersConnected = 0;
            strcpy(aux_poole.name, data[0]);
            strcpy(aux_poole.ip_poole, data[1]);
            strcpy(aux_poole.port_poole, data[2]);
            UniversalLinkedList_add(pooles, (void*) &aux_poole);

            asprintf(&aux_buffer, "New Poole added: %s\n", aux_poole.name);
            printF(aux_buffer);
            free(aux_buffer);

            free(data[0]);
            free(data[1]);
            free(data[2]);
            free(data[3]);
            free(data);
        }else if(strcmp(EXIT, data[count]) == 0){
            UniversalLinkedList_goToHead(pooles);
            while(UniversalLinkedList_isAtEnd(pooles) == 0){
                Poole* poole = (Poole*) UniversalLinkedList_get(pooles);
                if(strcmp(poole->name, data[0]) == 0){
                    asprintf(&aux_buffer, "\nUser deleted from %s\n\n", poole->name);
                    printF(aux_buffer);
                    free(aux_buffer);
                    poole->usersConnected--;
                    break;
                }
                UniversalLinkedList_next(pooles);
            }
            encodeMessage(CON_OK, NULL, buffer, 0x06);
            write(newsockfd, buffer, 256);
            free(data[0]);
            free(data[1]);
            free(data);
        }else if(strcmp(DISCONNECT, data[count]) == 0){
            UniversalLinkedList_goToHead(pooles);
            while(UniversalLinkedList_isAtEnd(pooles) == 0){
                Poole* poole = (Poole*) UniversalLinkedList_get(pooles);
                if(strcmp(poole->name, data[0]) == 0){
                    asprintf(&aux_buffer, "%s disconnected.\n\n", poole->name);
                    printF(aux_buffer);
                    free(aux_buffer);
                    free(poole->ip_poole);
                    free(poole->port_poole);
                    free(poole->name);
                    free(poole);
                    UniversalLinkedList_remove(pooles);
                    break;
                }
                UniversalLinkedList_next(pooles);
            }
            encodeMessage(CON_OK, NULL, buffer, 0x06);
            write(newsockfd, buffer, 256);
            free(data[0]);
            free(data[1]);
            free(data);
        }else{
            encodeMessage(CON_KO, NULL, buffer, 0x06);
            write(newsockfd, buffer, 256);
            free(data[0]);
            free(data[1]);
            free(data);
        }
        close(newsockfd);
    }
    return NULL;
}

/***********************************************
*
* @Finalitat: Funció que s'executa en un thread. S'encarrega de gestionar les peticions dels clients Bowman.
* @Parametres: in: void
* @Retorn:  void
*
************************************************/
void* socketB(){
    int sockfd = createServer(discovery.portB);
    char buffer[256];
    while(1){
        char *aux_buffer;
        int newsockfd = acceptClient(sockfd);
        memset(buffer, '\0', 256);
        read(newsockfd, buffer, 256);
        int count = 0;
        char** data_bowman = decodeMessage(buffer, &count, BOWMAN);
        
        UniversalLinkedList_goToHead(pooles);
        if(UniversalLinkedList_isAtEnd(pooles) == 1){
            memset(buffer, '\0', 256);
            encodeMessage(CON_KO, NULL, buffer, 0x01);
            write(newsockfd, buffer, 256);
            continue;
        }
        asprintf(&aux_buffer, "\n%s asked for a Poole.\n", data_bowman[0]);
        printF(aux_buffer);
        free(aux_buffer);
        Poole* poole = (Poole*) UniversalLinkedList_get(pooles);
        int usersConnected = poole->usersConnected;
        UniversalLinkedList_next(pooles);
        while(UniversalLinkedList_isAtEnd(pooles) == 0){
            Poole* aux_poole = (Poole*) UniversalLinkedList_get(pooles);
            if(aux_poole->usersConnected < usersConnected){
                usersConnected = aux_poole->usersConnected;
                poole = aux_poole;
            }
            UniversalLinkedList_next(pooles);
        }
        poole->usersConnected++;
        
        asprintf(&aux_buffer, "%s connected to %s\n\n", data_bowman[0], poole->name);
        printF(aux_buffer);
        free(aux_buffer);

        char* data_poole[3];
        data_poole[0] = (char*) malloc(sizeof(char) * strlen(poole->name)+1);
        data_poole[1] = (char*) malloc(sizeof(char) * strlen(poole->ip_poole)+1);
        data_poole[2] = (char*) malloc(sizeof(char) * strlen(poole->port_poole)+1);
        strcpy(data_poole[0], poole->name);
        strcpy(data_poole[1], poole->ip_poole);
        strcpy(data_poole[2], poole->port_poole);

        char *message = joinByChar(data_poole, 3, '&');
        encodeMessage(CON_OK, message, buffer, 0x01);
        free(message);
        write(newsockfd, buffer, 256);
        
        free(data_poole[0]);
        free(data_poole[1]);
        free(data_poole[2]);
        free(data_bowman[0]);
        free(data_bowman[1]);
        free(data_bowman);
        
    }
    return NULL;
}

/***********************************************
*
* @Finalitat: Funcio principal del programa. Encarregada de llegir el fitxer de configuracio i crear els threads.
* @Parametres: in: argc: Nombre d'arguments.
*              in: argv: Array d'arguments.
* @Retorn: 0 si tot ha anat be, 1 altrament.
*
************************************************/
int main(int argc, char *argv[]){

    SEM_constructor(&print_mutex);
    SEM_init(&print_mutex, 1);

    if(argc != 2){
        printF("Error: Wrong number of arguments\n");
        SEM_destructor(&print_mutex);
        return 1;
    }

    signal(SIGINT, sigintHandler);
    pooles = UniversalLinkedList_create(sizeof(Poole));

    fd_reader = open(argv[1], O_RDONLY);
    if(fd_reader < 0){
        UniversalLinkedList_destroy(&pooles);
        printF(ERROR_FILE);
        SEM_destructor(&print_mutex);
        return ERROR;
    }

    
    readFile();
    close(fd_reader);

    if (pthread_create(&thread_pooles, NULL, socketB, NULL) != 0) {
        printF(ERROR_THREAD);
        SEM_destructor(&print_mutex);
        return ERROR;
    }
    if (pthread_create(&thread_bowmans, NULL, socketP, NULL) != 0) {
        printF(ERROR_THREAD); 
        SEM_destructor(&print_mutex);
        return ERROR;
    }
    pthread_detach(thread_pooles);
    pthread_detach(thread_bowmans);
    
    printF("Discovery started successfully\n\n");
    pause();

    return 0;
}
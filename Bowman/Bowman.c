/***********************************************
*
* @Proposit: Client Bowman program que permet descarregar cançons i llistes de reproducció.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 14/10/2023
* @Data ultima modificacio: 8/1/2024
*
************************************************/
#define _GNU_SOURCE

#include "../Libraries/Config.h"
#include "../Libraries/TextUtilLib.h"
#include "../Libraries/userInterface.h"
#include "../Libraries/Comunication.h"
#include "../Libraries/semaphore_v2.h"
 

User user;
char *command;
int sockfd_poole;
LinkedList downloads;
semaphore mutex;
semaphore print_mutex;
pthread_t poole_thread;


/***********************************************
*
* @Finalitat: Printar per pantalla el missatge que es passa per parametre
* @Parametres: in: x: missatge a printar
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
* @Finalitat:
* @Parametres:
* @Retorn:
*
************************************************/
void freeMemory(User user){
    free(user.user_name);
    free(user.folder_name);
    free(user.ip_discovery_user);
    free(user.port_discovery_user);
}

/***********************************************
*
* @Finalitat: Printa les descarregues que hi ha en el sistema tant si estan completades com si no
* @Parametres: in: user: usuari que ha fet la comanda
* @Retorn: void
*
************************************************/
void checkDownloads(User *user){
    char *aux_buffer;
    int i = 0;
    if(user->num_downloads == 0){
        printF(NO_DOWNLOADS);
    }else{
        SEM_wait(&mutex);
        UniversalLinkedList_goToHead(downloads);
        while(UniversalLinkedList_isAtEnd(downloads) == 0){
            Download *download = (Download *) UniversalLinkedList_get(downloads);
            int percentage = (int) (((float) download->current_bytes / (float) download->total_bytes) * 100);
            int current_position = (int) (((float) download->current_bytes / (float) download->total_bytes) * 20);
            asprintf(&aux_buffer, "%d. %s [%.*s>%.*s] %d%%\n", i+1, download->file_name, current_position, "====================", 20 - current_position, "                    ", percentage);
            printF(aux_buffer);
            free(aux_buffer);
            i++;
            UniversalLinkedList_next(downloads);
        }
        SEM_signal(&mutex);
        printF("\n");
    }
}

/***********************************************
*
* @Finalitat: Elimina les descarregues que ja estan completades
* @Parametres: in: user: usuari que ha fet la comanda
* @Retorn: void
*
************************************************/
void clearDownloads(User *user){
    if(user->num_downloads == 0){
        printF(NO_DOWNLOADS);
        return;
    }
    SEM_wait(&mutex);
    UniversalLinkedList_goToHead(downloads);
    while(UniversalLinkedList_isAtEnd(downloads) == 0){
        Download *download = (Download *) UniversalLinkedList_get(downloads);
        if(download->completed == 1){
            free(download->file_name);
            free(download->md5sum);
            free(download->id);
            if(download->playlist_name != NULL){
                free(download->playlist_name);
            }
            free(download);
            UniversalLinkedList_remove(downloads);
            UniversalLinkedList_goToHead(downloads);
            user->num_downloads--;
        }else{
            UniversalLinkedList_next(downloads);
        }
    }
    if(user->num_downloads == 0){
        UniversalLinkedList_destroy(&downloads);
        SEM_destructor(&mutex);

        SEM_signal(&mutex);
        printF(NO_DOWNLOADS);
    }else{
        SEM_signal(&mutex);
        checkDownloads(user);
    }
}

/***********************************************
*
* @Finalitat: Escolta els missatges que envia el poole i els gestiona
* @Parametres: in: arg: usuari que ha fet login
* @Retorn: void
*
************************************************/
void *pooleListener(void *arg){
    User *user = ((User *) arg);
    char buffer[256];
    memset(buffer, '\0', 256);
    

    while (1){
        read(sockfd_poole, buffer, 256);
        int count = 0;
        char **decodedMessage = decodeMessage(buffer, &count, POOLE);

        if(strcmp(decodedMessage[0], CON_OK) == 0){
            switch (buffer[0]){
            case 0x06:
                free(decodedMessage[0]);
                free(decodedMessage);
                return NULL;
                break;
            case 0x04:
                if(user->num_downloads == 0){
                    downloads = UniversalLinkedList_create(sizeof(Download));
                    SEM_constructor(&mutex);
                    SEM_init(&mutex, 1);
                }
                break;
            default:
                break;
            }
        }else if(strcmp(decodedMessage[0], CON_KO) == 0){
            switch (buffer[0]){
            case 0x06:
                printF(ERROR_LOGOUT);
                break;
            case 0x04:
                printF(DOWNLOAD_NOT_FOUND);
                break;
            default:

                break;
            }
        }else if(strcmp(decodedMessage[0], SONGS_RESPONSE) == 0){
            printF(ERROR_SONGS);
        }else if(strcmp(decodedMessage[0], PLAYLISTS_RESPONSE) == 0){
            printF(ERROR_PLAYLISTS);
        }else if(strcmp(decodedMessage[0], DISCONNECT) == 0){
            printF(POOLE_DISCONNECTED);
            user->connected = 0;
            user->reconnected = 1;
            close(sockfd_poole);
            free(decodedMessage[0]);
            free(decodedMessage);
            pthread_detach(poole_thread);
            return NULL;
        }else if(strcmp(decodedMessage[count], SONGS_NUMBER) == 0){
            char *aux_buffer;
            char buffer2[256];
            asprintf(&aux_buffer, "\rThere are %d songs available for download:\n", atoi(decodedMessage[1]));
            printF(aux_buffer);
            free(aux_buffer);
            int numberOfBuffers = atoi(decodedMessage[0]);
            char *final_message = (char *) malloc(sizeof(char));
            for(int k = 0;k<numberOfBuffers;k++){
                read(sockfd_poole, buffer2, 256);
                joinBuffers(&final_message, buffer2, k, numberOfBuffers, SONGS_RESPONSE);
            }
            int count2;
            char **decodedMessage2 = decodeMessage(final_message, &count2, POOLE);
            for(int i = 0;i<count2;i++){
                asprintf(&aux_buffer, "%d. %s\n", (i+1), decodedMessage2[i]);
                printF(aux_buffer);
                free(aux_buffer);
            }
            
            free(final_message);
            for(int i = 0; i < count2+1; i++){
                free(decodedMessage2[i]);
            }
            free(decodedMessage2);

            free(decodedMessage[count]);
            printF("\n$ ");

        }else if(strcmp(decodedMessage[count], PLAYLISTS_NUMBER) == 0){
            char *aux_buffer;
            char buffer2[256];

            int numberOfBuffers = atoi(decodedMessage[0]);
            char *final_message = (char *) malloc(sizeof(char));
            for(int k = 0;k<numberOfBuffers;k++){
                read(sockfd_poole, buffer2, 256);
                joinBuffers(&final_message, buffer2, k, numberOfBuffers, PLAYLISTS_RESPONSE);
            }
            
            int count2;
            char **decodedMessage2 = decodeMessage(final_message, &count2, POOLE);
            asprintf(&aux_buffer, "\rThere are %d lists available for download:\n", count2);
            printF(aux_buffer);
            free(aux_buffer);
            for(int i = 0;i<count2;i++){
                int count2 = 0;
                char **aux_message = splitByChar(decodedMessage2[i], '&', &count2);
                asprintf(&aux_buffer, "%d. %s\n", (i+1), aux_message[0]);
                printF(aux_buffer);
                free(aux_buffer);
                for(int j = 1;j<count2;j++){
                    asprintf(&aux_buffer, "\t%c. %s\n", 'a' +(j-1), aux_message[j]);
                    printF(aux_buffer);
                    free(aux_buffer);
                }
                
                for(int j = 0; j < count2; j++){
                    free(aux_message[j]);
                }
                free(aux_message);
            }
            printF("\n$ ");
            
            free(final_message);
            for(int i = 0; i < count2+1; i++){
                free(decodedMessage2[i]);
            }
            free(decodedMessage2);
            free(decodedMessage[count]);

        }else if(strcmp(decodedMessage[count], FILE_DATA) == 0){
            SEM_wait(&mutex);
            char *aux_buffer;
            UniversalLinkedList_goToHead(downloads);
            while(UniversalLinkedList_isAtEnd(downloads) == 0){
                Download *download = (Download *) UniversalLinkedList_get(downloads);
                if((strcmp(download->id, decodedMessage[0]) == 0) && (download->completed == 0)){
                    int bytes_to_write = 0;
                    if(((int) (download->total_bytes - download->current_bytes)) >= (int) (256 - 3 - strlen(decodedMessage[0])-strlen(decodedMessage[count])-1)){
                        bytes_to_write = 256 - 3 - strlen(decodedMessage[0]) - strlen(decodedMessage[count])-1;
                        write(download->fd_file, buffer+ (256-bytes_to_write), bytes_to_write);
                    }else{
                        bytes_to_write = download->total_bytes - download->current_bytes; 
                        write(download->fd_file, buffer+ (3 + strlen(decodedMessage[0]) + strlen(decodedMessage[count])+1), bytes_to_write);
                    }
                    download->current_bytes += bytes_to_write;

                    if(((download->current_bytes == download->total_bytes) && download->completed == 0) && download->total_bytes != 0){
                        char md5_file_downloaded[33];
                        if(download->playlist_name != NULL){
                            calculateMD5(download->file_name , md5_file_downloaded, user->folder_name, download->playlist_name);
                        }else{
                            calculateMD5(download->file_name , md5_file_downloaded, user->folder_name, NULL);
                        }
                        if(strcmp(md5_file_downloaded, download->md5sum) == 0){
                            sendMessage(sockfd_poole, CHECK_OK, NULL, 0x05);
                            asprintf(&aux_buffer, "\r%s%s", download->file_name, SUCCESS_DOWNLOAD);
                            printF(aux_buffer);
                            free(aux_buffer);
                        }else{
                            sendMessage(sockfd_poole, CHECK_KO, NULL, 0x05);
                            asprintf(&aux_buffer, "\r%s%s", download->file_name, DOWNLOAD_FAILED);
                            printF(aux_buffer);
                            free(aux_buffer);
                        }
                        download->completed = 1;
                        close(download->fd_file);                
                    }
                }
                UniversalLinkedList_next(downloads);
            }
            usleep(10);
            SEM_signal(&mutex);
            free(decodedMessage[count]);

        }else if(strcmp(decodedMessage[count], NEW_FILE) == 0){
            user->num_downloads++;
            char *file_path;
            Download *new_download = (Download *) malloc(sizeof(Download));
            int tokens = 0;
            char **splitted_name = splitByChar(decodedMessage[0], '-', &tokens);

            if(tokens > 1){
                new_download->playlist_name = (char *) malloc(sizeof(char) * (strlen(splitted_name[0]) + 1));
                strcpy(new_download->playlist_name, splitted_name[0]);
                new_download->file_name = (char *) malloc(sizeof(char) * (strlen(splitted_name[1]) + 1));
                strcpy(new_download->file_name, splitted_name[1]);
                asprintf(&file_path, "%s/%s/%s", user->folder_name, PLAYLISTS_FOLDER, new_download->playlist_name);
                mkdir(file_path, 0777);
                free(file_path);
                asprintf(&file_path, "%s/%s/%s/%s", user->folder_name, PLAYLISTS_FOLDER, new_download->playlist_name, new_download->file_name);
            }else{
                new_download->playlist_name = NULL;
                new_download->file_name = (char *) malloc(sizeof(char) * (strlen(decodedMessage[0]) + 1));
                strcpy(new_download->file_name, decodedMessage[0]);
                asprintf(&file_path, "%s/%s/%s", user->folder_name, SONGS_FOLDER, new_download->file_name);
            }
            for(int i = 0; i < tokens; i++){
                free(splitted_name[i]);
            }
            free(splitted_name);

            new_download->total_bytes = atoi(decodedMessage[1]);
            new_download->md5sum = (char *) malloc(sizeof(char) * (strlen(decodedMessage[2]) + 1));
            strcpy(new_download->md5sum, decodedMessage[2]);
            new_download->id = (char *) malloc(sizeof(char) * (strlen(decodedMessage[3]) + 1));
            strcpy(new_download->id, decodedMessage[3]);

            new_download->fd_file = open(file_path, O_CREAT | O_WRONLY, 0666);
            if(new_download->fd_file < 0){
                free(file_path);
                printF(ERROR_FILE);
                return NULL;
            }
            new_download->current_bytes = 0;
            new_download->completed = 0;
            new_download->initialized = 1;
            SEM_wait(&mutex);
            UniversalLinkedList_add(downloads, (void*) new_download);
            SEM_signal(&mutex);
            printF(DOWNLOAD_START);
            free(new_download);
            free(file_path);
            free(decodedMessage[count]);
        }

        for(int i = 0; i < count; i++){
            free(decodedMessage[i]);
        }
        free(decodedMessage);
    }
    return NULL;
}

/***********************************************
*
* @Finalitat: Gestiona les comandes que es poden fer al programa
* @Parametres: in: command: comanda que ha fet l'usuari
               in: user: usuari que ha fet la comanda
* @Retorn: int: 1 si s'ha fet logout, 0 altrament
*
************************************************/
int manageCommand(char *command, User *user){
    toUpperCase(command);
    char **split_command;
    int num_tokens;

    split_command = splitByChar(command, ' ', &num_tokens);

    if(strcmp(split_command[0], DOWNLOAD) == 0){
        if(num_tokens == 2){
            if(user->connected==1){
                if(strlen(split_command[1])>4 && strcmp(split_command[1]+strlen(split_command[1])-4, ".MP3") == 0){
                    sendMessage(sockfd_poole, DOWNLOAD_SONG, split_command[1], 0x03);
                }else{
                    sendMessage(sockfd_poole, DOWNLOAD_LIST, split_command[1], 0x03);
                }
            }else{
                printF(ERROR_CONNECT);
            }
        }else{
            printF(ERROR_COMMAND);
        }
        
    }else if(strcmp(split_command[0], CONNECT) == 0 && num_tokens == 1 && user->connected == 0){
        if(connectUser(user, &sockfd_poole) == 0){
            printF(ERROR_CONNECT);
            if(user->reconnected == 1){
                SEM_destructor(&mutex);
            }
            SEM_destructor(&print_mutex);
            free(split_command[0]);
            free(split_command);
            return 1;
        }else{
            printF(user->user_name);
            printF(SUCCESS_CONNECT);

            pthread_create(&poole_thread, NULL, pooleListener, (void *) user);

            sendMessage(sockfd_poole,NEW_BOWMAN, user->user_name, 0x01);
        }
    }else if(strcmp(split_command[0], LOGOUT) == 0 && num_tokens == 1){
        if(user->connected==0 && user->reconnected == 0){
            for(int i = 0; i<num_tokens; i++){
                free(split_command[i]);
            }
            free(split_command);
            printF(SUCCESS_LOGOUT);
            return 1;
        }
        SEM_destructor(&mutex);
        SEM_destructor(&print_mutex);
        disconnectUser(user, EXIT, sockfd_poole, &downloads);
        for (int i = 0; i<num_tokens; i++){
            free(split_command[i]);
        }   
        free(split_command);
        
        pthread_join(poole_thread, NULL);
        printF(SUCCESS_LOGOUT);
        close(sockfd_poole);
        user->connected = 0;
        return 1;
    }else if(strcmp(split_command[0], LIST) == 0){
        if(user->connected==1){
            if(num_tokens == 2){
                if(strcmp(split_command[1], SONGS) == 0){
                    sendMessage(sockfd_poole, LIST_SONGS, NULL, 0x02);

                }else if(strcmp(split_command[1], PLAYLISTS) == 0){
                    sendMessage(sockfd_poole, LIST_PLAYLISTS, NULL, 0x02);
                }else{
                    printF(ERROR_COMMAND);
                }
            }else{
                printF(ERROR_COMMAND);
            }
            
        }else{
            printF(ERROR_CONNECT);
        }
        
    }else if(num_tokens == 2 && strcmp(split_command[0], CHECK) == 0 && strcmp(split_command[1], DOWNLOADS) == 0){
        if(user->connected==1){
            checkDownloads(user);
        }else{
            printF(ERROR_CONNECT);
        }
    }else if(num_tokens == 2 && strcmp(split_command[0], CLEAR) == 0 && strcmp(split_command[1], DOWNLOADS) == 0){
        if(user->connected==1){
            clearDownloads(user);
        }else{
            printF(ERROR_CONNECT);
        }
    }else{
        printF(ERROR_COMMAND);
    }
    
    for (int i = 0; i<num_tokens; i++){
        free(split_command[i]);
    }
    free(split_command);
    return SUCCESS;
}

/***********************************************
*
* @Finalitat: Funcio que s'executa quan es fa ctrl+c, allibera la memoria i fa un raise del ctrl+c
* @Parametres: in: signum: senyal que ha rebut
* @Retorn: void
*
************************************************/
void ending(){
    manageCommand("LOGOUT\0", &user);
    freeMemory(user);
    free(command);
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}

/***********************************************
*
* @Finalitat: Llegeix el fitxer de configuracio i guarda les dades en la estructura user
* @Parametres: in: user: estructura on es guardaran les dades
               in: fd_reader: file descriptor del fitxer de configuracio
* @Retorn: int: 0 si tot ha anat be, 1 altrament
* 
************************************************/
int readConfig(User *user, int fd_reader){
    readText(&(user->user_name), fd_reader, '\n');
    eliminateChar(user->user_name, '&');
    readText(&(user->folder_name), fd_reader, '\n');
    readText(&(user->ip_discovery_user), fd_reader, '\n');
    readText(&user->port_discovery_user, fd_reader, '\n');
    user->connected = 0;
    user->reconnected = 0;
    user->num_downloads = 0;
    close(fd_reader);
    return SUCCESS;
}

/***********************************************
*
* @Finalitat: Printa per pantalla les dades de l'usuari
* @Parametres: in: user: usuari que ha fet login
* @Retorn: void
*
************************************************/
void showUser(User *user){
    char *auxBuffer;
    
    asprintf(&auxBuffer, "\n%s user initialized", user->user_name);
    printF(auxBuffer);
    free(auxBuffer);

    printF("\n\nFile read correctly:");

    asprintf(&auxBuffer, "\nUser - %s", user->user_name);
    printF(auxBuffer);
    free(auxBuffer);

    asprintf(&auxBuffer, "\nDirectory - %s", user->folder_name);
    printF(auxBuffer);
    free(auxBuffer);

    asprintf(&auxBuffer, "\nIP - %s", user->ip_discovery_user);
    printF(auxBuffer);
    free(auxBuffer);

    asprintf(&auxBuffer, "\nPort - %s\n\n", user->port_discovery_user);
    printF(auxBuffer);
    free(auxBuffer);
}

/***********************************************
*
* @Finalitat: Funcio que demana les comandes a l'usuari i les gestiona amb la funcio manageCommand
* @Parametres:  void
* @Retorn: void
*
************************************************/
void userInterface(){
    showUser(&user);

    while(1){
        printF("$ ");
        readText(&command, 0, '\n');
        if(strcmp(command, "\0") != 0){
            if(manageCommand(command, &user) == 1){
                free(command);
                break;
            }
        }
        free(command);
    }
}

/***********************************************
*
* @Finalitat: Funcio main del programa executa tot el programa
* @Parametres: in: argc: nombre d'arguments que s'han passat
               in: argv: arguments que s'han passat
* @Retorn: int: 0 si tot ha anat be, 1 altrament
*
************************************************/
int main(int argc, char const *argv[]) {
    signal(SIGINT, ending);
    SEM_constructor(&print_mutex);
    SEM_init(&print_mutex, 1);

    if(argc != 2){
        printF(ERROR_ARGUMENTS);
        SEM_destructor(&print_mutex);
        return ERROR;
    }

    int fd_reader = open(argv[1], O_RDONLY);
    if (fd_reader < 0) {
        printF(ERROR_FILE);
        SEM_destructor(&print_mutex);
        return ERROR;
    }

    if(readConfig(&user, fd_reader) == ERROR){
        freeMemory(user);
        SEM_destructor(&print_mutex);
        return 1;
    }

    userInterface();

    freeMemory(user);

    return 0;
}

/***********************************************
*
* @Proposit: Servidor Poole encarregat de gestionar les peticions dels bowmans i enviar les dades que necessiten
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 14/10/2023
* @Data ultima modificacio: 8/1/2024
*
************************************************/

#define _GNU_SOURCE


#include "../Libraries/Config.h"
#include "../Libraries/Sockets.h"
#include "../Libraries/TextUtilLib.h"
#include "../Libraries/Comunication.h"
#include "../Libraries/UniversalLinkedList.h"
#include "../Libraries/pooleInterface.h"
#include "../Libraries/semaphore_v2.h"

pthread_mutex_t bowmansMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t monolit_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
    char *song_name;
    int number_of_downloads;
}Song;

Poole server;
LinkedList bowmans;
semaphore print_mutex;
int fd_monolit;
int ending = 0;
int pipefd[2];
int sockfd_poole;

typedef struct{
    User bowman;
    char *song_name;
    char *playlist_name;
}DownloadSongThread;

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
* @Finalitat: Llegeix el fitxer de configuracio i guarda les dades del servidor
* @Parametres: in: server: punter a l'estructura on es guardaran les dades del servidor
               in: file_name: nom del fitxer de configuracio
* @Retorn: 0 si tot ha anat be, 1 si hi ha hagut algun error
*
************************************************/
int readServer(Poole *server, char *file_name){
    int fd_reader = open(file_name, O_RDONLY);
    if (fd_reader < 0) {
        printF(FILE_ERROR);
        return 1;
    }
    printF("Reading configuration file\n");
    readText(&(server->name), fd_reader, '\n');
    readText(&(server->folder_name), fd_reader, '\n');
    readText(&(server->ip_discovery), fd_reader, '\n');
    readText(&(server->port_discovery), fd_reader, '\n');
    readText(&(server->ip_poole), fd_reader, '\n');
    readText(&(server->port_poole), fd_reader, '\n');
    server->usersConnected = 0;
    close(fd_reader);
    
    return 0;
}

/***********************************************
*
* @Finalitat: Allibera la memoria de l'estructura del servidor
* @Parametres: in: server: punter a l'estructura del servidor
* @Retorn: void
*
************************************************/
void freeServer(Poole *server){
    free(server->name);
    free(server->folder_name);
    free(server->ip_discovery);
    free(server->port_discovery);
    free(server->ip_poole);
    free(server->port_poole);
}

/***********************************************
*
* @Finalitat: Elimina un usuari de la llista de bowmans
* @Parametres: in: bowmans: llista de bowmans
               in: user_name: nom de l'usuari a eliminar
* @Retorn: void
*
************************************************/
void removeUser(LinkedList bowmans, char *user_name){
    UniversalLinkedList_goToHead(bowmans);
    while(UniversalLinkedList_isAtEnd(bowmans) == 0){
        User *bowman = (User *) UniversalLinkedList_get(bowmans);
        if(strcmp(bowman->user_name, user_name) == 0){
            free(bowman->user_name);
            free(bowman);
            UniversalLinkedList_remove(bowmans);
            return;
        }
        UniversalLinkedList_next(bowmans);
    }
}

/***********************************************
*
* @Finalitat: Tanca el process fill alliberant la memoria i tancant els fds
* @Parametres: in: signal: senyal que ha rebut el process
* @Retorn: void
*
************************************************/
void signalHandlerChild(){
    signal(SIGINT, SIG_DFL);
    close(pipefd[0]);
    close(fd_monolit);
    freeServer(&server);
    raise(SIGINT);
}

/***********************************************
*
* @Finalitat: Tanca el process pare alliberant la memoria, tanca els fds i envia un missatge de desconnexio al discovery
* @Parametres: in: signal: senyal que ha rebut el process
* @Retorn: void
*
************************************************/
void sigintHandler(){
    signal(SIGINT, SIG_DFL);
    close(pipefd[1]);
    SEM_destructor(&print_mutex);
    char buffer[256];
    char *aux_buffer;
    asprintf(&aux_buffer, "\n%s is disconnecting from the system...\n", server.name);
    printF(aux_buffer);
    free(aux_buffer);
    ending = 1;
    int num_bowmans = 0;
    int sockfd_discovery = connectDiscovery(server.ip_discovery, server.port_discovery);
    if(sockfd_discovery >= 0){
        memset(buffer, '\0', 256);
        encodeMessage(DISCONNECT, server.name, buffer, 0x06);
        write(sockfd_discovery, buffer, 256);
        close(sockfd_discovery);
    }
    memset(buffer, '\0', 256);
    encodeMessage(DISCONNECT, NULL, buffer, 0x07);
    if(bowmans != NULL) {
        UniversalLinkedList_goToHead(bowmans);
        while(UniversalLinkedList_isAtEnd(bowmans) == 0){
            User *bowman = (User *) UniversalLinkedList_get(bowmans);
            num_bowmans++;
            write(bowman->poole_sockfd, buffer, 256);
            
            free(bowman->user_name);
            close(bowman->poole_sockfd);
            free(bowman);
            UniversalLinkedList_remove(bowmans);
            if(UniversalLinkedList_isAtEnd(bowmans) == 0){
                UniversalLinkedList_goToHead(bowmans);
            }
        }
        UniversalLinkedList_destroy(&bowmans);
    }
    freeServer(&server);
	close(sockfd_poole);
    while(1){
        if(ending == (num_bowmans + 1)){
            break;
        }
    }
    wait(NULL);
    raise(SIGINT);
}

/***********************************************
*
* @Finalitat: Descarrega de cançons cap als bowmans. Es pot executar en paral·lel al tractar-se de threads
* @Parametres: in: arg: estructura que conte el nom de la canço i el nom del bowman
* @Retorn: void
*
************************************************/
void *downloadSong(void *arg){
    DownloadSongThread *downloadSongThread = (DownloadSongThread *) arg;
    char buffer[256];
    
    char *aux_buffer;
    int fd_song;
    char *song_name = NULL;
    int size_buffer = 256 - 3 - strlen(FILE_DATA)-1;
    char buffer_file[size_buffer];
    if(downloadSongThread->playlist_name != NULL){
        asprintf(&aux_buffer, "%s/%s/%s/%s", server.folder_name, PLAYLISTS_FOLDER, downloadSongThread->playlist_name, downloadSongThread->song_name);
    }else{
        song_name = checkExistingSong(downloadSongThread->song_name, server.folder_name);
        asprintf(&aux_buffer, "%s/%s/%s", server.folder_name, SONGS_FOLDER, song_name);
    }
    

    fd_song = open(aux_buffer, O_RDONLY);
    free(aux_buffer);
    if(fd_song < 0){
        printF("Error opening song\n");
        pthread_exit(NULL);
    }
    int size = lseek(fd_song, 0, SEEK_END);
    lseek(fd_song, 0, SEEK_SET);
   
    int random_number = rand() % 1000;
    char *random_number_string;

    asprintf(&random_number_string, "%d", random_number);
    char md5[33];
    if(downloadSongThread->playlist_name != NULL){
        calculateMD5(downloadSongThread->song_name, md5, server.folder_name, downloadSongThread->playlist_name);
        asprintf(&aux_buffer, "%s-%s&%d&%s&%s",downloadSongThread->playlist_name, downloadSongThread->song_name, size, md5, random_number_string);
    }else{
        calculateMD5(song_name, md5, server.folder_name, NULL);
        asprintf(&aux_buffer, "%s&%d&%s&%s", song_name, size, md5, random_number_string);
        free(song_name);
    }
    memset(buffer, '\0', 256);
    encodeMessage(NEW_FILE, aux_buffer, buffer, 0x04);

    pthread_mutex_lock(&writeMutex);
    write(downloadSongThread->bowman.poole_sockfd, buffer, 256);
    pthread_mutex_unlock(&writeMutex);
    free(aux_buffer);

    pthread_mutex_lock(&monolit_mutex);
    asprintf(&aux_buffer, "%s", downloadSongThread->song_name);
    write(downloadSongThread->bowman.pipe_monolit, aux_buffer, strlen(aux_buffer));
    pthread_mutex_unlock(&monolit_mutex);
    free(aux_buffer);

    ssize_t bytesRead;

    while((bytesRead = read(fd_song, buffer_file, size_buffer-strlen(random_number_string))) > 0){
        char buffer[256];
        memset(buffer, '\0', 256);
        asprintf(&aux_buffer, "%s&", random_number_string);
        encodeMessage(FILE_DATA, aux_buffer, buffer, 0x04);
        for(int i = 0;i<bytesRead;i++){
            buffer[i + 3 + strlen(FILE_DATA) + strlen(aux_buffer)] = buffer_file[i];    
        }
        pthread_mutex_lock(&writeMutex);
        write(downloadSongThread->bowman.poole_sockfd, buffer, 256);
        usleep(10000);
		pthread_mutex_unlock(&writeMutex);
        free(aux_buffer);
    }

    close(fd_song);
    free(random_number_string);
    free(downloadSongThread->song_name);
    if(downloadSongThread->playlist_name != NULL){
        free(downloadSongThread->playlist_name);
    }
    free(downloadSongThread);
    return NULL;
}

/***********************************************
*
* @Finalitat: Funcio que s'executa en un thread per gestionar les peticions dels bowmans
* @Parametres: in: argv: estructura que conte la informacio del bowman
* @Retorn: void
*
************************************************/
void *userConnection(void *argv){
    User *bowman = ((User *) argv);
    char buffer[256];
    while(1){
        memset(buffer, '\0', 256);
        read(bowman->poole_sockfd, buffer, 256);

        pthread_mutex_lock(&bowmansMutex);
        if(ending > 0){
            free(bowman);
            ending++;
            pthread_mutex_unlock(&bowmansMutex);
            return NULL;
        }
        pthread_mutex_unlock(&bowmansMutex);

        int count = 0;
        char **decodedMessage = decodeMessage(buffer, &count, BOWMAN);

        if(count == 1){
            if(strcmp(decodedMessage[0], LIST_SONGS) == 0){
                char *aux_buffer;
                char buffer2[256];
                asprintf(&aux_buffer, "\nNew request - %s requires the list of songs.\n", bowman->user_name);
                printF(aux_buffer);
                free(aux_buffer);

                int songs_count = 0;
                char **songs = getSongList(server.folder_name, &songs_count);
                if(songs != NULL && songs_count > 0){
                    asprintf(&aux_buffer, "Sending song list to %s\n", bowman->user_name);
                    printF(aux_buffer);
                    free(aux_buffer);
                    char *message2 = joinByChar(songs, songs_count, '&');
                    int number_of_messages;
                    char **buffer_messages = separateMessage(message2, 256 - 3 - strlen(SONGS_RESPONSE), &number_of_messages);

                    char *number_of_buffers;
                    asprintf(&number_of_buffers, "%d&%d", number_of_messages, songs_count);
                    memset(buffer2, '\0', 256);
                    encodeMessage(SONGS_NUMBER, number_of_buffers, buffer2, 0x02);
                    pthread_mutex_lock(&writeMutex);
                    write(bowman->poole_sockfd, buffer2, 256);
                    pthread_mutex_unlock(&writeMutex);
                    free(number_of_buffers);
                    for(int i = 0;i<number_of_messages;i++){
                        memset(buffer2, '\0', 256);
                        encodeMessage(SONGS_RESPONSE, buffer_messages[i], buffer2, 0x02);
                        pthread_mutex_lock(&writeMutex);
                        write(bowman->poole_sockfd, buffer2, 256);
                        pthread_mutex_unlock(&writeMutex);
                        free(buffer_messages[i]);
                    }
                    
                    free(buffer_messages);
                    free(message2);
                    for(int i = 0;i<songs_count;i++){
                        free(songs[i]);
                    }
                    free(songs);
                }else{
                    free(songs);
                    memset(buffer2, '\0', 256);
                    encodeMessage(SONGS_RESPONSE, NULL, buffer2, 0x01);
                    pthread_mutex_lock(&writeMutex);
                    write(bowman->poole_sockfd, buffer2, 256);
                    pthread_mutex_unlock(&writeMutex);
                }
                
            }else if(strcmp(decodedMessage[0], LIST_PLAYLISTS) == 0){
                char *aux_buffer;
                asprintf(&aux_buffer, "\nNew request - %s requires the list of playlists.\n", bowman->user_name);
                printF(aux_buffer);
                free(aux_buffer);

                char buffer2[256];
                int *songs_count = NULL;
                int playlists_count = 0;
                char ***playlists = getPlaylists(server.folder_name, &playlists_count, &songs_count);
                if(playlists != NULL){
                    asprintf(&aux_buffer, "Sending playlist list to %s\n", bowman->user_name);
                    printF(aux_buffer);
                    free(aux_buffer);

                    char **messages = (char **) malloc(sizeof(char *) * playlists_count);

                    for(int i = 0;i<playlists_count;i++){
                        messages[i] = joinByChar(playlists[i], songs_count[i]+1, '&');
                    }
                    char *message = joinByChar(messages, playlists_count, '#');
                    int number_of_messages;
                    char **buffer_messages = separateMessage(message, 256 - 3 - strlen(PLAYLISTS_RESPONSE), &number_of_messages);
                    char *number_of_buffers;
                    asprintf(&number_of_buffers, "%d", number_of_messages);
                    memset(buffer2, '\0', 256);
                    encodeMessage(PLAYLISTS_NUMBER, number_of_buffers, buffer2, 0x02);
                    pthread_mutex_lock(&writeMutex);
                    write(bowman->poole_sockfd, buffer2, 256);
                    pthread_mutex_unlock(&writeMutex);
                    free(number_of_buffers);
                    for(int i = 0;i<number_of_messages;i++){
                        memset(buffer2, '\0', 256);
                        encodeMessage(PLAYLISTS_RESPONSE, buffer_messages[i], buffer2, 0x02);
                        pthread_mutex_lock(&writeMutex);
                        write(bowman->poole_sockfd, buffer2, 256);
                        pthread_mutex_unlock(&writeMutex);
                        free(buffer_messages[i]);
                    }

                    for(int i = 0;i<playlists_count;i++){
                        free(messages[i]);
                    }
                    free(messages);
                    free(message);
                    free(buffer_messages);

                    for(int i = 0;i<playlists_count;i++){
                        for(int j = 0;j<songs_count[i]+1;j++){
                            free(playlists[i][j]);
                        }
                        free(playlists[i]);
                    }
                    free(playlists);
                    free(songs_count);

                }else{
                    memset(buffer, '\0', 256);
                    encodeMessage(PLAYLISTS_RESPONSE, NULL, buffer, 0x02);
                    pthread_mutex_lock(&writeMutex);
                    write(bowman->poole_sockfd, buffer, 256);
                    pthread_mutex_unlock(&writeMutex);
                }
            }else if(strcmp(decodedMessage[0], CHECK_OK) == 0){
                char *aux_buffer;
                asprintf(&aux_buffer, "Song uploaded successfully to %s\n\n", bowman->user_name);
                printF(aux_buffer);
                free(aux_buffer);
            }else if(strcmp(decodedMessage[0], CHECK_KO) == 0){
                char *aux_buffer;
                asprintf(&aux_buffer, "Song uploaded failed to %s\n\n", bowman->user_name);
                printF(aux_buffer);
                free(aux_buffer);
            }else if(strcmp(decodedMessage[count], DOWNLOAD_SONG) == 0){
                char *aux_buffer;
                asprintf(&aux_buffer, "\nNew request - %s requires the song %s.\n", bowman->user_name, decodedMessage[0]);
                printF(aux_buffer);
                free(aux_buffer);
                

                char *song_name = checkExistingSong(decodedMessage[0], server.folder_name);
                if(song_name != NULL){
                    memset(buffer, '\0', 256);
                    encodeMessage(CON_OK, NULL, buffer, 0x04);
                    pthread_mutex_lock(&writeMutex);
                    write(bowman->poole_sockfd, buffer, 256);
                    pthread_mutex_unlock(&writeMutex);

                    free(song_name);
                    asprintf(&aux_buffer, "Sending song %s to %s\n", decodedMessage[0], bowman->user_name);
                    printF(aux_buffer);
                    free(aux_buffer);
                    DownloadSongThread *downloadSongThread = (DownloadSongThread *) malloc(sizeof(DownloadSongThread));
                    downloadSongThread->bowman = *bowman;
                    downloadSongThread->song_name = (char *) malloc(sizeof(char) * (strlen(decodedMessage[0]) + 1));
                    downloadSongThread->playlist_name = NULL;
                    strcpy(downloadSongThread->song_name, decodedMessage[0]);
                    pthread_t download_thread;
                    pthread_create(&download_thread, NULL, downloadSong, (void *) downloadSongThread);
                    pthread_detach(download_thread);
                }else{
                    asprintf(&aux_buffer, "Song %s not found\n", decodedMessage[0]);
                    printF(aux_buffer);
                    free(aux_buffer);

                    memset(buffer, '\0', 256);
                    encodeMessage(CON_KO, NULL, buffer, 0x04);
                    pthread_mutex_lock(&writeMutex);
                    write(bowman->poole_sockfd, buffer, 256);
                    pthread_mutex_unlock(&writeMutex);
                }

                free(decodedMessage[count]);
            }else if(strcmp(decodedMessage[count], DOWNLOAD_LIST) == 0){
                char *aux_buffer;
                asprintf(&aux_buffer, "\nNew request - %s requires the playlist %s.\n", bowman->user_name, decodedMessage[0]);
                printF(aux_buffer);
                free(aux_buffer);
                int count2;
                char **songs = getSongListFromPlaylist(decodedMessage[0], server.folder_name, &count2);
                if(songs != NULL){
                    asprintf(&aux_buffer, "Sending the playlist %s to %s. A total of %d songs will be sent\n\n", decodedMessage[0], bowman->user_name, count2-1);
                    printF(aux_buffer);
                    free(aux_buffer);
                    memset(buffer, '\0', 256);
                    encodeMessage(CON_OK, NULL, buffer, 0x04);
                    pthread_mutex_lock(&writeMutex);
                    write(bowman->poole_sockfd, buffer, 256);
                    pthread_mutex_unlock(&writeMutex);

                    for(int i = 0;i<count2-1;i++){
                        DownloadSongThread *downloadSongThread = (DownloadSongThread *) malloc(sizeof(DownloadSongThread));
                        downloadSongThread->playlist_name = (char *) malloc(sizeof(char) * (strlen(decodedMessage[0]) + 1));
                        strcpy(downloadSongThread->playlist_name, songs[0]);
                        downloadSongThread->bowman = *bowman;
                        downloadSongThread->song_name = (char *) malloc(sizeof(char) * (strlen(songs[i+1]) + 1));
                        strcpy(downloadSongThread->song_name, songs[i+1]);
                        pthread_t download_thread;
                        pthread_create(&download_thread, NULL, downloadSong, (void *) downloadSongThread);
                        pthread_detach(download_thread);
                    }
                    for(int i = 0;i<count2;i++){
                        free(songs[i]);
                    }
                    free(songs);
                }else{
                    asprintf(&aux_buffer, "Playlist %s not found\n", decodedMessage[0]);
                    printF(aux_buffer);
                    free(aux_buffer);

                    memset(buffer, '\0', 256);
                    encodeMessage(CON_KO, NULL, buffer, 0x04);
                    pthread_mutex_lock(&writeMutex);
                    write(bowman->poole_sockfd, buffer, 256);
                    pthread_mutex_unlock(&writeMutex);
                }

                free(decodedMessage[count]);
            }else if(strcmp(decodedMessage[count], EXIT) == 0){
                int sockfd_discovery = connectDiscovery(server.ip_discovery, server.port_discovery);
                if(sockfd_discovery >= 0){
                    server.usersConnected--;
                    removeUser(bowmans, decodedMessage[0]);
                    memset(buffer, '\0', 256);
                    encodeMessage(EXIT, server.name, buffer, 0x06);
                    pthread_mutex_lock(&writeMutex);
                    write(sockfd_discovery, buffer, 256);
                    pthread_mutex_unlock(&writeMutex);
                    read(sockfd_discovery, buffer, 256);
                    char **decodedMessage_2 = decodeMessage(buffer, &count, POOLE);
                    if(strcmp(decodedMessage_2[0], CON_OK) == 0){
                        disconnect(bowman->poole_sockfd);
                        printF("\nUser disconected: ");
                        printF(decodedMessage[0]);
                        printF("\n");
                    }else{
                        encodeMessage(CON_KO, NULL, buffer, 0x06);
                        pthread_mutex_lock(&writeMutex);
                        write(bowman->poole_sockfd, buffer, 256);
                        pthread_mutex_unlock(&writeMutex);
                    }
                    free(decodedMessage_2[0]);
                    free(decodedMessage_2);
                    close(sockfd_discovery);
                }
                free(decodedMessage[1]);
                free(decodedMessage[0]);
                free(decodedMessage);
                close(bowman->poole_sockfd);
                free(bowman);
                pthread_exit(NULL);
            }
        }
        for(int i = 0;i<count; i++){
            free(decodedMessage[i]);
        }
        free(decodedMessage);
    }
}

/***********************************************
*
* @Finalitat: Va rebent les peticions de connexio dels bowmans i les va gestionant
* @Parametres: in: pipe_monolit: pipe per comunicar-se amb el process fill
* @Retorn: void
*
************************************************/
void connectionBowman(int pipe_monolit){
    char *aux_buffer;
    char buffer[256];
    bowmans = UniversalLinkedList_create(sizeof(User));
    sockfd_poole = createServer(server.port_poole);
    if(sockfd_poole < 0){
        UniversalLinkedList_destroy(&bowmans);
        SEM_destructor(&print_mutex);
        signal(SIGINT, SIG_DFL);       
        freeServer(&server); 
        printF("Error creating the server\nWaiting for a CTRL+C to close the program\n");
        while(1){
            sleep(1);
        }
        return;
    }
    while(1){
        int poole_fd = acceptClient(sockfd_poole);
        read(poole_fd, buffer, 256);
        int count = 0;
        char **decodedMessage = decodeMessage(buffer, &count, BOWMAN);

        User *aux_bowman = (User *)malloc(sizeof(User));
        aux_bowman->pipe_monolit = pipe_monolit;
        aux_bowman->poole_sockfd = poole_fd;
        aux_bowman->user_name = (char *)malloc(sizeof(char) * (strlen(decodedMessage[0]) + 1));
        strcpy(aux_bowman->user_name, decodedMessage[0]);
        UniversalLinkedList_add(bowmans, (void *)aux_bowman);
        server.usersConnected++;


        asprintf(&aux_buffer, "\nNew user connected: %s.\n", aux_bowman->user_name);
        printF(aux_buffer);
        free(aux_buffer);

        pthread_t thread;
        pthread_create(&thread, NULL, userConnection, (void *) aux_bowman);
        pthread_detach(thread);

        free(decodedMessage[0]);
        free(decodedMessage[1]);
        free(decodedMessage);
    }
}

/***********************************************
*
* @Finalitat: Escriu les estadistiques de les cançons al fitxer stats.txt
* @Parametres: in: fd_monolit: fd del fitxer stats.txt
               in: buffer: nom de la canço
* @Retorn: void
*
************************************************/
void writeStats(int fd_monolit, char *buffer) {
    char *aux_buffer;
    int chars_read = 0;
    char *song_name = NULL;
    int number_of_downloads = 0;

    song_name = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
    strcpy(song_name, buffer);

    lseek(fd_monolit, 0, SEEK_SET);

    while (1) {
        chars_read = readText(&aux_buffer, fd_monolit, ' ');

        if (chars_read == -1) {
            free(aux_buffer);
            asprintf(&aux_buffer, "%s 1\n", song_name);
            write(fd_monolit, aux_buffer, strlen(aux_buffer));
            free(aux_buffer);
            break;
        }

        if (strcmp(aux_buffer, song_name) == 0){
            free(aux_buffer);
            chars_read = readText(&aux_buffer, fd_monolit, '\n');
            number_of_downloads = atoi(aux_buffer);
            number_of_downloads++;
            lseek(fd_monolit, -1 * (strlen(aux_buffer) + strlen(song_name) + 2), SEEK_CUR);
            free(aux_buffer);
            asprintf(&aux_buffer, "%s %d\n",song_name, number_of_downloads);
            write(fd_monolit, aux_buffer, strlen(aux_buffer));
            free(aux_buffer);
            break;
        }else{
            free(aux_buffer);
            chars_read = readText(&aux_buffer, fd_monolit, '\n');
            if (chars_read == -1) {
                free(aux_buffer);
                lseek(fd_monolit, 0, SEEK_END);
                asprintf(&aux_buffer, "%s 1\n", song_name);
                write(fd_monolit, aux_buffer, strlen(aux_buffer));
                free(aux_buffer);
                break;
            }
            free(aux_buffer);
        }
    }
    free(song_name);
}

/***********************************************
*
* @Finalitat: Main program del servidor poole
* @Parametres: in: argc: nombre d'arguments
               in: argv: arguments
* @Retorn: 0 si tot ha anat be, 1 si hi ha hagut algun error
*
************************************************/
int main(int argc, char const *argv[]){
    SEM_constructor(&print_mutex);
    SEM_init(&print_mutex, 1);

    if(argc != 2){
        printF("Error en los argumentos\n");
        SEM_destructor(&print_mutex);
        return 1;
    }
    char buffer[256];
    char *file_name = (char *) argv[1];
    if(readServer(&server, file_name)){
        SEM_destructor(&print_mutex);
        return 1;
    }

    signal(SIGINT, sigintHandler);
    printF(CONNECTING);

    int sockfd = connectDiscovery(server.ip_discovery, server.port_discovery);
    if(sockfd < 0){
        freeServer(&server);
        SEM_destructor(&print_mutex);
        return 1;
    }
    printF(CONNECTED);

    memset(buffer, '\0', 256);
    char* data[3];
    data[0] = server.name;
    data[1] = server.ip_poole;
    data[2] = server.port_poole;
    char * message =  joinByChar(data, 3, '&');
    encodeMessage(NEW_POOLE,message, buffer, 0x01);
    free(message);

    write(sockfd, buffer, 256);

    close(sockfd);

    pipe(pipefd);
    pid_t pid = fork();
    
    switch (pid){
        case -1:
            printF("Error forking the process\n");
            freeServer(&server);
            SEM_destructor(&print_mutex);
            return 1;
            break;
        case 0:
            signal(SIGINT, signalHandlerChild);
            close(pipefd[1]);
            fd_monolit = open("stats.txt", O_RDWR  | O_CREAT, 0666);
            if(fd_monolit < 0){
                printF("Error opening stats.txt\n");
                SEM_destructor(&print_mutex);
                close(pipefd[0]);
                return 1;
            }
            while(1){
                memset(buffer, '\0', 256);
                read(pipefd[0], buffer, 256);
                writeStats(fd_monolit, buffer);
            }
            break;
        default:
            close(pipefd[0]);
            
            connectionBowman(pipefd[1]);
            break;
    }

    freeServer(&server);
    return 0;
}

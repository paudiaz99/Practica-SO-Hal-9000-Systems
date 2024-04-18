/***********************************************
*
* @Proposit: Implementa les funcions de la llibreria TextUtilLib.h que permeten la manipulacio de textos i strings.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 14/10/2023
* @Data ultima modificacio: 28/12/2023
*
************************************************/
#include "TextUtilLib.h"
#include "Config.h"

/***********************************************
*
* @Finalitat: Separa el text donat un separador i retorna un array de strings amb les paraules separades.
* @Parametres: in: text = el text a separar.
*              in: separator = el separador.
*              in/out: count = el nombre de paraules separades.
* @Retorn: 
*
************************************************/
char** splitByChar(char* text, char separator, int* count) {
    int i = 0;
    int k = 0;
    int size = 0;
    int size_aux = 0;
    int lastWord = 0;
    char** result = (char**)malloc(sizeof(char*));
    while (text[i] != '\0') {
        if (text[i] == separator) {
            while(text[i] == separator){
                i++;
                if(text[i] == '\0'){
                    lastWord = 1;
                    break;
                }
            }
            if(lastWord){
                break;
            }
            result = (char**)realloc(result, (size + 1) * sizeof(char*));
            result[size] = (char*)malloc(sizeof(char) * (size_aux + 1));
            for (int l = 0; l < size_aux; ++l) {
                if(text[k + l] != ' '){
                    result[size][l] = text[k + l];
                }else{
                    result[size][l] = '\0';
                }
            }
            result[size][size_aux] = '\0';
            size++;
            size_aux = 0;
            k = i;
            
        } else {
            size_aux++;
            i++;
        }
    }
    result = (char**)realloc(result, (size + 1) * sizeof(char*));
    result[size] = (char*)malloc(sizeof(char) * (size_aux + 1));
    for (int l = 0; l < size_aux; ++l) {
        if(text[k + l] != ' '){
            result[size][l] = text[k + l];
        }else{
            result[size][l] = '\0';
        }
    }
    result[size][size_aux] = '\0';
    size++;
    *count = size;

    return result;
}

/***********************************************
*
* @Finalitat: Llegeix un text fins a trobar un separador i el retorna.
* @Parametres: in/out: destination = el text llegit.
*              in: fd_reader = el file descriptor del fitxer del qual es llegeix.
*              in: separator = el separador.
* @Retorn: int: 0 si s'ha llegit correctament, -1 altrament.
*
************************************************/
int readText(char** destination, int fd_reader, char separator) {
    char auxChar;
    int i = 0;
    
    *destination = (char*)malloc(sizeof(char));
    if (*destination == NULL) {
        return -1;
    }

    while (1) {
        int n_read = read(fd_reader, &auxChar, 1);

        if (n_read == 0) {
            (*destination)[i] = '\0'; 
            return -1;
        }

        if (auxChar == separator) {
            (*destination)[i] = '\0';
            break;
        } else {
            *destination = (char*)realloc(*destination, (i + 2) * sizeof(char));
            if (*destination == NULL) {
                return -1;
            }
            (*destination)[i] = auxChar;
            i++;
        }
    }

    return 0;
}

/***********************************************
*
* @Finalitat: Passa un text a majuscules.
* @Parametres: in/out: text = el text a passar a majuscules.
* @Retorn: void
*
************************************************/
void toUpperCase(char *text){
    int i = 0;
    while(text[i] != '\0'){
        if(text[i] >= 'a' && text[i] <= 'z'){
            text[i] =  (text[i] - 'a') + 'A';
        }
        i++;
    }
}

/***********************************************
*
* @Finalitat: Passa un text a minuscules.
* @Parametres: in/out: text = el text a passar a minuscules.
* @Retorn: void
*
************************************************/
void toLowerCase(char *text){
    int i = 0;
    while(text[i] != '\0'){
        if(text[i] >= 'A' && text[i] <= 'Z'){
            text[i] =  (text[i] - 'A') + 'a';
        }
        i++;
    }
}

/***********************************************
*
* @Finalitat: Elimina un caracter d'un text.
* @Parametres: in/out: text = el text del qual s'elimina el caracter.
* @Retorn: void
*
************************************************/
void eliminateChar(char *text, char character){

    for (int i = 0; text[i] != '\0'; ++i) {
        if (text[i] == character) {
            for (int j = i; text[j] != '\0'; ++j) {
                text[j] = text[j + 1];
            }
        }
    }
}

/***********************************************
*
* @Finalitat: Junta diferents arrays de strings en un de sol amb un separador entre ells.
* @Parametres: in: text = els arrays de strings a unir.
*              in: inputs = el nombre d'arrays de strings a unir.
*              in: separator = el separador.
* @Retorn: char*: el resultat de la unio dels arrays de strings.
*
************************************************/
char* joinByChar(char* text[], int inputs, char separator){
    char* result = (char*)malloc(sizeof(char));
    int result_size = 0;
    for (int i = 0; i < inputs; ++i) {
        int input_size = strlen(text[i]);
        result = (char*)realloc(result, (result_size + input_size + 1) * sizeof(char));
        for (int j = 0; j < input_size; ++j) {
            result[result_size + j] = text[i][j];
        }
        result[result_size + input_size] = separator;
        result_size += input_size + 1;
    }
    result[result_size - 1] = '\0';
    return result;
}

/***********************************************
*
* @Finalitat: Separa un text en diferents arrays de strings de tamany splitSize.
* @Parametres: in: message = el text a separar.
*              in: splitSize = el tamany de cada array de strings.
*              in/out: count = el nombre d'arrays de strings.
* @Retorn: char**: els arrays de strings.
* 
************************************************/
char **separateMessage(char *message, int splitSize, int *count){
    char **result = (char **) malloc(sizeof(char *));
    *count = 0;
    int i = 0;
    
    while (message[i] != '\0') {
        result = (char **) realloc(result, (*count + 1) * sizeof(char *));
        result[*count] = (char *) malloc(sizeof(char) * (splitSize + 1)); 

        int j;
        for (j = 0; j < splitSize && message[i] != '\0'; ++j) {
            result[*count][j] = message[i];
            i++;
        }
        result[*count][j] = '\0';

        (*count)++;
    }

    return result;
}

/***********************************************
*
* @Finalitat: Separa un text per caracters tantes vegades com es demani amb la variable times.
* @Parametres: in: text = el text a separar.
*              in: separator = el separador.
*              in/out: count = el nombre de paraules separades.
*              in: times = el nombre de vegades que es vol separar el text.
* @Retorn: char**: els arrays de strings.
*
************************************************/
char** splitByCharByTimes(char* text, char separator, int* count, int times){
    int i = 0;
    int k = 0;
    int size = 0;
    int size_aux = 0;
    int lastWord = 0;
    char** result = (char**)malloc(sizeof(char*));

    while (text[i] != '\0' && size < times) {
        if (text[i] == separator) {
            while (text[i] == separator) {
                i++;
                if (text[i] == '\0') {
                    lastWord = 1;
                    break;
                }
            }
            if (lastWord) {
                break;
            }
            result = (char**)realloc(result, (size + 1) * sizeof(char*));
            result[size] = (char*)malloc(sizeof(char) * (size_aux + 1));
            for (int l = 0; l < size_aux; ++l) {
                if (text[k + l] != ' ') {
                    result[size][l] = text[k + l];
                } else {
                    result[size][l] = '\0';
                }
            }
            result[size][size_aux] = '\0';
            size++;
            size_aux = 0;
            k = i;
        } else {
            size_aux++;
            i++;
        }
    }

    if (size < times) {
        result = (char**)realloc(result, (size + 1) * sizeof(char*));
        result[size] = (char*)malloc(sizeof(char) * (size_aux + 1));
        for (int l = 0; l < size_aux; ++l) {
            if (text[k + l] != ' ') {
                result[size][l] = text[k + l];
            } else {
                result[size][l] = '\0';
            }
        }
        result[size][size_aux] = '\0';
        size++;
    }

    *count = size;

    return result;
}



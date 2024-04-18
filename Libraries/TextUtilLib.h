/***********************************************
*
* @Proposit: Llibreria que conté les funcions necessaries per a la manipulacio de textos i strings.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 14/10/2023
* @Data ultima modificacio: 28/12/2023
*
************************************************/
#ifndef BUILD_H
#define BUILD_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/***********************************************
*
* @Finalitat: Separa el text donat un separador i retorna un array de strings amb les paraules separades.
* @Parametres: in: text = el text a separar.
*              in: separator = el separador.
*              in/out: count = el nombre de paraules separades.
* @Retorn: 
*
************************************************/
char** splitByChar(char *text, char separator, int *count);

/***********************************************
*
* @Finalitat: Llegeix un text fins a trobar un separador i el retorna.
* @Parametres: in/out: destination = el text llegit.
*              in: fd_reader = el file descriptor del fitxer del qual es llegeix.
*              in: separator = el separador.
* @Retorn: int: 0 si s'ha llegit correctament, -1 altrament.
*
************************************************/
int readText(char** destination, int fd_reader, char separator);

/***********************************************
*
* @Finalitat: Passa un text a majuscules.
* @Parametres: in/out: text = el text a passar a majuscules.
* @Retorn: void
*
************************************************/
void toUpperCase(char *text);

/***********************************************
*
* @Finalitat: Passa un text a minuscules.
* @Parametres: in/out: text = el text a passar a minuscules.
* @Retorn: void
*
************************************************/
void toLowerCase(char *text);

/***********************************************
*
* @Finalitat: Elimina un caracter d'un text.
* @Parametres: in/out: text = el text del qual s'elimina el caracter.
* @Retorn: void
*
************************************************/
void eliminateChar(char *text, char character);

/***********************************************
*
* @Finalitat: Junta diferents arrays de strings en un de sol amb un separador entre ells.
* @Parametres: in: text = els arrays de strings a unir.
*              in: inputs = el nombre d'arrays de strings a unir.
*              in: separator = el separador.
* @Retorn: char*: el resultat de la unio dels arrays de strings.
*
************************************************/
char* joinByChar(char* text[], int inputs, char separator);

/***********************************************
*
* @Finalitat: Separa un text en diferents arrays de strings de tamany splitSize.
* @Parametres: in: message = el text a separar.
*              in: splitSize = el tamany de cada array de strings.
*              in/out: count = el nombre d'arrays de strings.
* @Retorn: char**: els arrays de strings.
* 
************************************************/
char ** separateMessage(char *message, int splitSize, int *count);

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
char** splitByCharByTimes(char* text, char separator, int* count, int times);

#endif
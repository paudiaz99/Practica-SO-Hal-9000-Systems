/***********************************************
*
* @Proposit: Llibreria que permet crear una llista universal que pot guardar qualsevol tipus d'element.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 14/11/2023
* @Data ultima modificacio: 20/11/2023
*
************************************************/
#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_
#include "Config.h"


#define LIST_NO_ERROR 0
#define LIST_ERROR_FULL 1			
#define LIST_ERROR_EMPTY 2			
#define LIST_ERROR_MALLOC 3
#define LIST_ERROR_END 4


typedef void* Element;

typedef struct list_t* LinkedList;

/***********************************************
*
* @Finalitat: Crea una llista buida.
* @Parametres: in: element_size = la mida dels elements que es guardaran a la llista.
* @Retorn: LinkedList: la llista creada.
*
************************************************/
LinkedList UniversalLinkedList_create (size_t element_size);

/***********************************************
*
* @Finalitat: Afegeix un element a la llista.
* @Parametres: in/out: list = la llista a la que s'afegeix l'element.
*              in: element = l'element a afegir.
* @Retorn: void
*
************************************************/
void 	UniversalLinkedList_add (LinkedList list, Element element);

/***********************************************
*
* @Finalitat: Elimina un element de la llista.
* @Parametres: in/out: list = la llista de la que s'elimina l'element.
* @Retorn: void
*
************************************************/
void UniversalLinkedList_remove (LinkedList list);

/***********************************************
*
* @Finalitat: Funció que retorna l'element de la llista situat a la posicio actual.
* @Parametres: in: list = la llista de la que es vol obtenir l'element.
* @Retorn: Element: l'element de la llista situat a la posicio actual.
*
************************************************/
Element UniversalLinkedList_get (LinkedList list);

/***********************************************
*
* @Finalitat: Fa que la posicio actual de la llista sigui la primera.
* @Parametres: in/out: list = la llista de la que es vol obtenir l'element.
* @Retorn: void
*
************************************************/
void 	UniversalLinkedList_goToHead (LinkedList list);

/***********************************************
*
* @Finalitat: Fa que la posicio actual de la llista sigui la seguent.
* @Parametres: in/out: list = la llista de la que es vol obtenir l'element.
* @Retorn: void
*
************************************************/
void UniversalLinkedList_next (LinkedList list);

/***********************************************
*
* @Finalitat: Funció que retorna 1 si la posicio actual de la llista es la ultima i 0 en cas contrari.
* @Parametres: in: list = la llista de la que es vol obtenir l'element.
* @Retorn: int: 1 si la posicio actual de la llista es la ultima i 0 en cas contrari.
*
************************************************/
int UniversalLinkedList_isAtEnd (LinkedList list);

/***********************************************
*
* @Finalitat: Destruccio de la llista i alliberament de la memoria.
* @Parametres: in/out: list = la llista a destruir.
* @Retorn: void
*
************************************************/
void UniversalLinkedList_destroy (LinkedList* list);

/***********************************************
*
* @Finalitat: Retorna el codi d'error de la llista.
* @Parametres: in: list = la llista de la que es vol obtenir l'error.
* @Retorn: int: el codi d'error de la llista.
*
************************************************/
int	UniversalLinkedList_getErrorCode (LinkedList list);


#endif


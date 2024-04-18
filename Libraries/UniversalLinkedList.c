/***********************************************
*
* @Proposit: Implementa les funcions de la llibreria UniversalLinkedList.h que permeten la manipulacio de llistes universals. 
             De tal manera que es poden afegir elements de qualsevol tipus a la llista.
* @Autor/s: Pau Diaz i Joan Labal
* @Data creacio: 14/11/2023
* @Data ultima modificacio: 20/11/2023
*
************************************************/
#include <stdlib.h>					
#include "UniversalLinkedList.h"

typedef struct _Node {		
	Element element;
	struct _Node * next;
} Node;

struct list_t {
	int error;
	Node * head;
	Node * previous;
	size_t element_size;
};

/************************************************ 
* 
* @Finalitat: Crea una llista buida.
* @Parametres: in: element_size = la mida dels elements que es guardaran a la llista.
* @Retorn: LinkedList: la llista creada.
*
************************************************/

LinkedList UniversalLinkedList_create(size_t element_size) {
    LinkedList list = (LinkedList)malloc(sizeof(struct list_t));

    list->head = (Node*)malloc(sizeof(Node));
    if (NULL != list->head) {
        list->head->next = NULL;
        list->previous = list->head;
        list->element_size = element_size; 

        list->error = LIST_NO_ERROR;
    } else {
        list->error = LIST_ERROR_MALLOC;
    }

    return list;
}

/***********************************************
*
* @Finalitat: Afegeix un element a la llista.
* @Parametres: in/out: list = la llista a la que s'afegeix l'element.
*              in: element = l'element a afegir.
* @Retorn: void
*
************************************************/
void UniversalLinkedList_add(LinkedList list, Element element) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (NULL != new_node) {
        new_node->element = malloc(list->element_size);
        if (new_node->element != NULL) {
            memcpy(new_node->element, element, list->element_size);
            new_node->next = list->previous->next;

            list->previous->next = new_node;
            list->previous = new_node;
            list->error = LIST_NO_ERROR;
        } else {
            list->error = LIST_ERROR_MALLOC;
            free(new_node);
        }
    } else {
        list->error = LIST_ERROR_MALLOC;
    }
}

/***********************************************
*
* @Finalitat: Elimina un element de la llista.
* @Parametres: in/out: list = la llista de la que s'elimina l'element.
* @Retorn: void
*
************************************************/
void UniversalLinkedList_remove (LinkedList list) {
	Node* aux = NULL;
	if (UniversalLinkedList_isAtEnd (list)) {
		list->error = LIST_ERROR_END;
	}
	else {
		aux = list->previous->next;
		list->previous->next = list->previous->next->next;
		free(aux);
		list->error = LIST_NO_ERROR;
	}
}

/***********************************************
*
* @Finalitat: Funció que retorna l'element de la llista situat a la posicio actual.
* @Parametres: in: list = la llista de la que es vol obtenir l'element.
* @Retorn: Element: l'element de la llista situat a la posicio actual.
*
************************************************/
Element UniversalLinkedList_get (LinkedList list) {
	Element element;		
	if (UniversalLinkedList_isAtEnd (list)) {
		list->error = LIST_ERROR_END;
	}
	else {
		element = list->previous->next->element;
		list->error = LIST_NO_ERROR;
	}
	return element;
}

/***********************************************
*
* @Finalitat: Fa que la posicio actual de la llista sigui la primera.
* @Parametres: in/out: list = la llista de la que es vol obtenir l'element.
* @Retorn: void
*
************************************************/
void UniversalLinkedList_goToHead (LinkedList list) {
	list->previous = list->head;
}

/***********************************************
*
* @Finalitat: Fa que la posicio actual de la llista sigui la seguent.
* @Parametres: in/out: list = la llista de la que es vol obtenir l'element.
* @Retorn: void
*
************************************************/
void UniversalLinkedList_next (LinkedList list) {

	if (UniversalLinkedList_isAtEnd (list)) {
		list->error = LIST_ERROR_END;
	}
	else {
		list->previous = list->previous->next;

		list->error = LIST_NO_ERROR;
	}
}

/***********************************************
*
* @Finalitat: Funció que retorna 1 si la posicio actual de la llista es la ultima i 0 en cas contrari.
* @Parametres: in: list = la llista de la que es vol obtenir l'element.
* @Retorn: int: 1 si la posicio actual de la llista es la ultima i 0 en cas contrari.
*
************************************************/
int UniversalLinkedList_isAtEnd (LinkedList list) {
	return NULL == list->previous->next;
}

/***********************************************
*
* @Finalitat: Destruccio de la llista i alliberament de la memoria.
* @Parametres: in/out: list = la llista a destruir.
* @Retorn: void
*
************************************************/
void UniversalLinkedList_destroy (LinkedList* list) {
	Node* aux;
	while (NULL != (*list)->head) {
		aux = (*list)->head;
		(*list)->head = (*list)->head->next;
		free(aux);
	}
	(*list)->head = NULL;
	(*list)->previous = NULL;

	free(*list);
	*list = NULL;
}

/***********************************************
*
* @Finalitat: Retorna el codi d'error de la llista.
* @Parametres: in: list = la llista de la que es vol obtenir l'error.
* @Retorn: int: el codi d'error de la llista.
*
************************************************/
int	UniversalLinkedList_getErrorCode (LinkedList list) {
	return list->error;
}

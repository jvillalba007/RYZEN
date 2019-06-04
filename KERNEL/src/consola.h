#ifndef CONSOLA_H
#define CONSOLA_H

#include <shared/console.h>
#include "shared/utils.h"
#include <commons/string.h>
#include <pthread.h>
#include "config/configKernel.h"


/* 
    Tipo de dato: t_tabla
    Nodo con:
        - nombre de la tabla.
        - criterio de consistencia.
        - puntero al próximo nodo.
    Estos nodos serán parte de la lista de tablas-consistencias tendrá estos nodos
*/
typedef struct Nodo_tabla_consistencia{
    char* nombre_tabla;
    char* criterio_consistencia;
    struct Nodo_tabla_consistencia* siguiente;
} t_tabla;


/*
    Tipo de dato: t_memoria_del_pool
    Nodo con:
        - número de memoria.
        - ip.
        - puerto.
        - puntero al próximo nodo.
    Estos nodos serán parte de la lista de memorias del pool de memorias
*/
typedef struct t_memoria_del_pool{
    int numero_memoria;
    char* ip;
    char* puerto;
    int cant_inserts_ejecutados;
    int cant_selects_ejecutados;
    struct t_memoria_del_pool* siguiente;
} t_memoria_del_pool;


/*
    Tipo de dato: int
    Variable que alojará el número de memoria correspondiente a la única memoria con criterio de consistencia SC
*/
int numero_memoria_con_criterio_SC;


/*
    Tipo de dato: t_memoria
    Nodo con:
        - número de memoria.
        - puntero al próximo nodo.
    Estos nodos serán parte de dos listas:
        - la lista de memorias con criterio de consistencia SHC.
        - la lista de memorias con criterio de consistencia EC.
*/
typedef struct Nodo_memorias{
    int numero_memoria;
    struct Nodo_memorias* siguiente;
} t_memoria;


/*
    Tipo de dato: t_PCB
    Nodo con:
        - identificador del PCB.
        - lista de requests.
        - PC = próximo número de línea a ejecutar.
        - puntero al pŕóximo nodo
*/
typedef struct PCB_request {
    int id;
    struct nodo_request* lista_requests;
    int PC;
    struct PCB_request* siguiente;
} t_PCB;


    /*
        Tipo de dato: t_request
        Nodo con:
            - número de línea
            - request (código a ejecutar de una única línea)
            - instante en el que se inició la request (READY --> RUNNING).
            - instante en el que se finalizó la request (RUNNING --> EXIT).
    */
    typedef struct nodo_request {
        int numero_de_linea;
        char* request;
        time_t instante_inicial;
        time_t instante_final;
        struct nodo_request* siguiente;
    } t_request;


//  terminar de definir las listas de NEWs, READYs, EXECs y EXITs... para no perder la referencia de la request con las demás


void consola();
void procesar_comando(char*);
void notificar_error_sintactico_en_comando(void);
void notificar_error_sintactico_en_parametros(void);
void notificar_error_tipo_consistencia(void);

#endif
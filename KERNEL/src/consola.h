#ifndef CONSOLA_H
#define CONSOLA_H

#include <shared/console.h>
#include "shared/utils.h"
#include <commons/string.h>
#include <pthread.h>
#include "config/configKernel.h"
#include <time.h>


/* 
    Tipo de dato: t_tabla
    Nodo con:
        - nombre de la tabla.
        - criterio de consistencia.
    Estos nodos serán parte de la lista de tablas-consistencias tendrá estos nodos
*/
typedef struct tabla_consistencia{
    char* nombre_tabla;
    char* criterio_consistencia;
} t_tabla_consistencia;


/*
    Tipo de dato: t_memoria_del_pool
    Nodo con:
        - número de memoria.
        - ip.
        - puerto.
    Estos nodos serán parte de la lista de memorias del pool de memorias
*/
typedef struct t_memoria_del_pool{
    int numero_memoria;
    char* ip;
    char* puerto;
} t_memoria_ip_puerto;


/*
    Tipo de dato: int
    Variable que alojará el número de memoria correspondiente a la única memoria con criterio de consistencia SC
    (Nodo) que alojará el número de memoria correspondiente a las memorias con criterio de consistencia SHC
    (Nodo) que alojará el número de memoria correspondiente a las memorias con criterio de consistencia EC
*/
typedef int t_memoria;
memoria_SC t_memoria;


/*
    Tipo de dato: t_PCB
    Nodo con:
        - request/comando = lo ingresado por consola.
        - tipo de request = al ejecutar no es lo mismo ejecutar un request/comando simple o un request/comando compuesto
        - PC = próximo número de línea a ejecutar.
*/
typedef enum {
    REQUEST_SIMPLE,     // 0    //  cualquier request/comando que no sea RUN, será de tipo REQUEST_SIMPLE
    REQUEST_COMPUESTA   // 1    //  cualquier request/comando RUN, será de tipo REQUEST_COMPUESTA
} t_tipo_request;

typedef struct PCB_request {
    char* request_comando;
    t_tipo_request tipo_request;
    int PC;
} t_PCB;


/*
Las estructuras de las estadísticas las veremos más adelante.

Tanto el instante_inicial como el instante final tienen que ser de tipo time_t.
*/

void consola();
void procesar_comando(char*);
void notificar_error_sintactico_en_comando(void);
void notificar_error_sintactico_en_parametros(void);
void notificar_error_tipo_consistencia(void);

#endif
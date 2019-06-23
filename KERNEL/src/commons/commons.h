#ifndef COMMONS_H
#define COMMONS_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <pthread.h>
#include "shared/utils.h"

t_log* logger;
t_config* config;

typedef struct {
    char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	int MULTIPROCESAMIENTO;
	int SLEEP_EJECUCION;
	int METADATA_REFRESH;
	int QUANTUM;
} ck;

ck kernel_config;


typedef enum {
    SIMPLE,     // 0    //  cualquier request/comando que no sea RUN, será de tipo REQUEST_SIMPLE
    COMPUESTA   // 1    //  cualquier request/comando RUN, será de tipo REQUEST_COMPUESTA
} t_tipo_request;

/*
    Tipo de dato: t_PCB
        - request/comando = lo ingresado por consola.
        - tipo de request = al ejecutar no es lo mismo ejecutar un request/comando simple o un request/comando compuesto
        - PC = próximo número de línea a ejecutar.
*/
typedef struct{
	int id;
    char* request_comando;
    t_tipo_request tipo_request;
    int pc;
} t_PCB;


/*
    serán parte de la lista de tablas-consistencias tendrá estos nodos
*/
typedef struct{
    char* nombre_tabla;
    char* criterio_consistencia;
} t_tabla_consistencia;


/*
    serán parte de la lista de memorias del pool de memorias. TODO:confirmar definicion de estos atributos
*/
typedef struct{
    int numero_memoria;
    char* ip;
    char* puerto;
    char* criterio;
    bool activa;
} t_memoria_del_pool;

int id_pcbs; //para controlar los id de pcbs entrantes
int socket_memoria;
pthread_mutex_t sem_ejecutar; //mutex para obtener los pcb de los hilos de ejecucion


//LISTAS DE ESTADOS
t_list* l_pcb_nuevos;
t_list* l_pcb_listos;
t_list* l_pcb_ejecutando;
t_list* l_pcb_finalizados;

//LISTAS DE CRITERIOS DE MEMORIA memorias activas para redirigir las request segun criterio
t_list* l_criterio_SC;
t_list* l_criterio_SHC;
t_list* l_criterio_EC;

//LISTA DE MEMORIAS , estaran todas las memorias con las que alguna vez interactuo el sistema
t_list* l_memorias;

//LISTA DE HILOS PROCESADORES
t_list* l_procesadores;

//LISTA DE TABLAS
t_list* l_tablas;

int exit_global;


void inicializar_logs_y_configs(void);
void abrir_log(void);
void crear_config(void);
void leer_configs(void);
void loggear_inicio_logger(void);
void loggear_configs(void);

void liberar_kernel();//libera todos los pedidos de la memoria del proceso
void liberar_config();
void free_Pcb(t_PCB* pcb_borrar);
void free_memoria(t_memoria_del_pool* memoria_borrar);
void free_tabla(t_tabla_consistencia* tabla_borrar);
void terminar_hilos_procesadores();

#endif

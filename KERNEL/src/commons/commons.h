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
#include <shared/protocolo.h>
#include <signal.h>
#include <time.h>

t_log* logger;
t_config* config;

typedef struct {
    char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	int MULTIPROCESAMIENTO;
	int SLEEP_EJECUCION;
	int METADATA_REFRESH;
	int GOSSIPING_REFRESH;
	int QUANTUM;
} ck;

ck kernel_config;


typedef enum {
    SIMPLE,     // 0    //  cualquier request/comando que no sea RUN, será de tipo REQUEST_SIMPLE
    COMPUESTA   // 1    //  cualquier request/comando RUN, será de tipo REQUEST_COMPUESTA
} t_tipo_request;

/*
    representaicon de los pedidos al kernel
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
   Representacion de tabla en sistema
*/
typedef struct{
    char* nombre_tabla;
    char* criterio_consistencia;
    u_int8_t nro_particiones;
	u_int32_t tiempo_compactacion;
} t_tabla_consistencia;


/*
 Representacion de memoria en sistema
*/
typedef struct{
    int numero_memoria;
    char* ip;
    char* puerto;
    char* criterio;
    bool activa;
    int socket;
    float tiempo_select; //tiempo promedio que tarda un SELECT en ejecutarse en los últimos 30 segundos.
    float tiempo_insert; //tiempo promedio que tarda un INSERT en ejecutarse en los últimos 30 segundos.
    int cantidad_select; //Cantidad de SELECT ejecutados en los últimos 30 segundos.
    int cantidad_insert; // Cantidad de INSERT ejecutados en los últimos 30 segundos.
    int cantidad_carga; //Cantidad de INSERT / SELECT que se ejecutaron en esa memoria respecto de las operaciones totales.
} t_memoria_del_pool;

int operaciones_totales; //sumatoria de todas las operaciones que se ejecutaron en las memorias.
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

t_memoria_del_pool* obtener_memoria_random( t_list* memorias ); //obtiene memoria del pool para realizar el gossiping
t_list* get_memorias_activas( t_list* tabla_memorias ); //me da las memorias activas de una lista

void liberar_kernel();//libera todos los pedidos de la memoria del proceso
void liberar_config();
void free_Pcb(t_PCB* pcb_borrar);
void free_memoria(t_memoria_del_pool* memoria_borrar);
void free_tabla(t_tabla_consistencia* tabla_borrar);
void terminar_hilos_procesadores();
void liberar_memorias_gossiping(t_list *memorias);
void free_memoria_gossiping( pmemoria *memoria );
void free_tabla_describe( linea_create *linea ); //hace el free de los elementos recibidos en un describe

#endif

/*
 * memoria.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef MEMORIA_H_
	#define MEMORIA_H_
	#include "consola.h"
	#include <shared/socket.h>
	#include <shared/protocolo.h>
	#include <shared/utils.h>
	#include <pthread.h>
	#include <time.h>
	#include <signal.h>
	#include <inttypes.h>

pthread_t tid_server;
pthread_t tid_inotify;
pthread_t tid_journal;
pthread_t tid_gossiping;

pthread_mutex_t mutex;

#define ceiling(x,y) (((x) + (y) - 1) / (y))

void estructurar_memoria();/* inicializa todas las estructuras funcionales para la memoria */
void iniciar_memoria_contigua();
void iniciar_tabla_segmentos();
void iniciar_tabla_memorias(); //crea tabla de memorias e inicializa con los datos de archivo la tabla para el gossiping

int atender_request(int cliente, t_msg* msg); //funcion de atender request
int atender_kernel(int cliente, t_msg* msg); //gestiona cosas de kernel

void crear_servidor(); /* crea servidor de la memoria donde recibira las request de kernel y de otras meomrias */
void crear_cliente_lfs(); /* crea cliente para poder enviar las request hacia el LFS */

int tamanio_fila_Frames();

fila_TSegmentos*  crear_segmento( char *nombre_tabla ); //crea segmento y lo agrega a la lista de la tabla de segmentos
fila_TPaginas* crear_pagina(  fila_TSegmentos *segmento , char* frame , int8_t modificado ); //crea pagina y lo agrega a la tabla de paginas del segmento indicado
void actualizar_pagina( fila_TPaginas* pagina , linea_insert linea ); //actualiza los valores de la pagina y del frame con los nuevos valores
fila_TSegmentos* obtener_segmento( char *nombre_tabla );/* obtiene segmento con el mismo nombre de tabla, si no lo encuentra retorna NULL */
fila_TPaginas* obtener_pagina_segmento( fila_TSegmentos *segmento , u_int16_t key ); /* obtiene pagina de segmento segun la key si no la encuentra retorna null  */
int SPA_obtener_frame();
char* obtener_frame_libre();//me devuelve un marco libre, en el caso que la memoria este llena ejecutara el lru , en el caso que no le dara el puntero al inicio del marco
char* ejecutar_lru();
fila_Frames inicializar_fila_frame( linea_insert linea ); //inicializa la linea del frame desde un insert para escribir en memoria

void ejecutar_insert(linea_insert *linea); //ejecuta comando insert tanto como desde consola como desde una request de kernel
fila_TPaginas * ejecutar_select( linea_select* linea ); //ejecuta comando select tanto desde consola como request de kernel. devuelve la pagina para devolver la info
int ejecutar_drop( char* tabla );



/******** REQUEST LFS *******/
void enviar_describe_lfs( char *tabla );
void enviar_create_lfs( linea_create linea_c );
linea_response_select* enviar_select_lfs( linea_select *linea ); //request de select a LFS devuelve puntero del struct o null si lfs no pudo resolverla
int enviar_drop_lfs( char *tabla );

void hilo_journal();//corre cada x tiempo proceso journal
void journal();
void hilo_gossiping();//corre cada x tiempo proceso gossiping
void gossiping();
void agregar_memorias_gossiping( t_list *memorias_seed );//recibe tabla de intercambio de gossip y verifica si la agrega o no a la tabla de memorias.
t_list* get_memorias_activas( t_list* tabla_memorias );

void inotify_config(void);

#endif /* MEMORIA_H_ */

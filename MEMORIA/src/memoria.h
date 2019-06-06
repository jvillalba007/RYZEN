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
	#include <pthread.h>
	#include <time.h>

#define ceiling(x,y) (((x) + (y) - 1) / (y))

void estructurar_memoria();/* inicializa todas las estructuras funcionales para la memoria */
void iniciar_memoria_contigua();
void iniciar_tabla_segmentos();

void atender_request(void* cliente_socket); //hilo/funcion de atender request
void atender_kernel(int* cliente); //gestiona cosas de kernel

void crear_servidor(); /* crea servidor de la memoria donde recibira las request de kernel y de otras meomrias */
void crear_cliente_lfs(); /* crea cliente para poder enviar las request hacia el LFS */
void ejecutar_gossiping(); /* ejecuta proceso de gossiping cada x tiempo */

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
void ejecutar_drop( char* tabla );



/******** REQUEST LFS *******/
void enviar_create_lfs( linea_create linea_c );
linea_response_select* enviar_select_lfs( linea_select *linea ); //request de select a LFS devuelve puntero del struct o null si lfs no pudo resolverla
void enviar_drop_lfs( char *tabla );
void enviar_insert_lfs( linea_insert linea ); //seguramente la utilice el journal


#endif /* MEMORIA_H_ */

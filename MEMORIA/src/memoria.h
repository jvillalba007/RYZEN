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
fila_TPaginas* crear_pagina(  fila_TSegmentos *segmento , char* frame ); //crea pagina y lo agrega a la tabla de paginas del segmento indicado
void actualizar_pagina( fila_TPaginas* pagina , linea_insert linea ); //actualiza los valores de la pagina y del frame con los nuevos valores
fila_TSegmentos* obtener_segmento( char *nombre_tabla );/* obtiene segmento con el mismo nombre de tabla, si no lo encuentra retorna NULL */
fila_TPaginas* obtener_pagina_segmento( fila_TSegmentos *segmento , u_int16_t key ); /* obtiene pagina de segmento segun la key si no la encuentra retorna null  */

char* obtener_frame_libre();//me devuelve un marco libre, en el caso que la memoria este llena ejecutara el lru , en el caso que no le dara el puntero al inicio del marco
char* ejecutar_lru();
fila_Frames inicializar_fila_frame( linea_insert linea ); //inicializa la linea del frame desde un insert para escribir en memoria

#endif /* MEMORIA_H_ */

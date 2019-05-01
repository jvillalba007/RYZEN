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
	#include <pthread.h>

typedef struct {
	int32_t timestamp;
	u_int16_t key;
	char* value;
} fila_TFrames;

typedef struct {
	int32_t numero_pagina;
	fila_TFrames *frame;
	int8_t modificado;
	int32_t ultimo_uso; //ultimo tiempo de uso. Lo usa el LRU
	//TODO verificar si hay que agregar el puntero al segmento
} fila_TPaginas;

typedef struct {
	char nombre_tabla[255];
	t_list* paginas;
} fila_TSegmentos;


t_list* tabla_frames = list_create();
t_list* tabla_paginas = list_create();
t_list* tabla_segmentos = list_create();
int socketClienteLfs;

uint8_t maximo_value = 10; //TAMANIO MAXIMO DEL VALUE RECIBIDO EN BYTES POR LFS
int cantidad_frames;


void estructurar_memoria();/* inicializa todas las estructuras funcionales para la memoria */
void iniciar_tabla_frames();
void iniciar_tabla_paginas();

void crear_servidor(); /* crea servidor de la memoria donde recibira las request de kernel y de otras meomrias */
void crear_cliente_lfs(); /* crea cliente para poder enviar las request hacia el LFS */
void ejecutar_gossiping(); /* ejecuta proceso de gossiping cada x tiempo */

int tamanio_fila_TFrames();

#endif /* MEMORIA_H_ */

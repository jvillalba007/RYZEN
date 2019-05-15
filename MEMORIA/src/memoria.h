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

void estructurar_memoria();/* inicializa todas las estructuras funcionales para la memoria */
void iniciar_memoria_contigua();
void iniciar_tabla_segmentos();

void crear_servidor(); /* crea servidor de la memoria donde recibira las request de kernel y de otras meomrias */
void crear_cliente_lfs(); /* crea cliente para poder enviar las request hacia el LFS */
void ejecutar_gossiping(); /* ejecuta proceso de gossiping cada x tiempo */

int tamanio_fila_Frames();

#endif /* MEMORIA_H_ */

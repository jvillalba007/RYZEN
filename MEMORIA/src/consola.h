/*
 * consola.h
 *
 *  Created on: 25 abr. 2019
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

	#include <shared/console.h>
	#include <commons/string.h>
	#include <pthread.h>

void consola(); /* consola de MEMORIA */
void consola_procesar_comando(char* linea); /* Procesamiento de comando de MEMORIA */

#endif /* CONSOLA_H_ */

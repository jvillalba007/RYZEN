#ifndef CONSOLA_H
#define CONSOLA_H

	#include <shared/console.h>
	#include <commons/string.h>
	#include <pthread.h>
	#include "config/configKernel.h"

void consola();
void procesar_comando(char*);

#endif
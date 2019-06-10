#ifndef CONSOLA_H
#define CONSOLA_H

#include <shared/console.h>
#include "shared/utils.h"
#include <commons/string.h>
#include <pthread.h>
#include "commons/commons.h"
#include <time.h>


void consola();
void procesar_comando(char*);
void notificar_error_sintactico_en_comando(void);
void notificar_error_sintactico_en_parametros(void);
void notificar_error_tipo_consistencia(void);

void crear_pcb (char* string_codigo, t_tipo_request tipo); //permite crear pcb y psaarlo a lista de nuevos

#endif

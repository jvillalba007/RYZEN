#ifndef CONSOLA_H
#define CONSOLA_H

#include <shared/console.h>
#include "shared/utils.h"
#include "shared/socket.h"
#include <commons/string.h>
#include <pthread.h>
#include "commons/commons.h"
#include <time.h>


void consola ();
void procesar_comando (char*);
void crear_pcb (char* string_codigo, t_tipo_request tipo);

t_memoria_del_pool* obtener_memoria (int);
void enviar_journal_lista_memorias (t_list* ); //recibe una lista y envia un journal a las memorias que esten activas de esa lista
bool es_comando_conocido (char**);
bool es_correcta_cantidad_parametros (char*, int);
bool es_comando_planificable (char*);
bool es_string (char*, char*);

void notificar_error_comando_incorrecto (void);
void notificar_error_comando_cantidad_parametros (void);
void notificar_error_tipo_consistencia (void);

#endif

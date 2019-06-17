#ifndef KERNEL_H
#define KERNEL_H

#include <pthread.h>
#include "commons/commons.h"
#include "consola.h"
#include <shared/socket.h>
#include <shared/console.h>
#include "shared/utils.h"

int exit_global = 0;

void conectar_memoria();
void inicializar_kernel(); //inicializa las listas que se utilizaran en kernel
void ejecutar_procesador(); //ejecuta en un hilo los pcb

t_PCB* obtener_pcb_ejecutar(); // obtiene un pcb a ejecutar de la listo de ready.
void ejecutar_linea( char *linea ); //recibe una linea y la envia a ejecutar a alguna memoria
char* obtener_nombre_tabla( char* linea ); //me devuelve el nombre de la tabla a partir de una linea de request
t_tabla_consistencia *obtener_tabla( char* n_tabla ); //devuelve la tabla a partir del nombre
t_memoria_del_pool *obtener_memoria_criterio( t_tabla_consistencia* tabla ); //a partir de una tabla me indica que memoria va a realizar la request segun criterio
void ejecutar_linea_memoria( t_memoria_del_pool* memoria , char* linea ); //ejecuta en memoria la linea

void crear_procesadores();
void ejecutar_describe();
char* obtenerLinea(int pc, FILE* programa);





#endif

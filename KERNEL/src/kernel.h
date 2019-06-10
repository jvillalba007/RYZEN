#ifndef KERNEL_H
#define KERNEL_H

#include <pthread.h>
#include "commons/commons.h"
#include "consola.h"
#include <shared/socket.h>
#include <shared/console.h>
#include "shared/utils.h"



void conectar_memoria();
void inicializar_kernel(); //inicializa las listas que se utilizaran en kernel
void ejecutar(); //ejecuta en un hilo los pcb

t_PCB* obtener_pcb_ejecutar(); // obtiene un pcb a ejecutar de la listo de ready.

void liberar_kernel();
void crear_procesadores();
#endif

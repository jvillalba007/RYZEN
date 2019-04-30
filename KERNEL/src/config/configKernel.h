#ifndef CONFIGKERNEL_H
#define CONFIGKERNEL_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <string.h>

t_log* logger;
t_config* config;

typedef struct {
    char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	int MULTIPROCESAMIENTO;
	int SLEEP_EJECUCION;
	int METADATA_REFRESH;
	int QUANTUM;
} ck;

ck kernel_config;

void abrir_log(void);
void crear_config(void);
void leer_configs(void);
void loggear_inicio_logger(void);
void loggear_configs(void);
void liberar_kernel_config(ck);

#endif
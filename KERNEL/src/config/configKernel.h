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


void abrirLog(void);
void loggearInicioLogger(void);
void crearConfig(void);
void leerConfigs(void);
void loggearConfigs(void);
void liberar_kernel_config(ck kernel_config);

#endif
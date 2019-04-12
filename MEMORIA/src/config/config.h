/*
 * config.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <string.h>

#define pathCFG "MEMORIA.CFG"
#define pathLOG "MEMORIA.LOG"

typedef struct {
	char* puerto_mem;
	char* ip_LFS;
	char* puerto_LFS;
	char** ip_SEEDS;
	char** puerto_SEEDS;
	int retardo_mem;
	int retardo_fs;
	int tam_mem;
	int retardo_journal;
	int retardo_gossiping;
	int memory_number;
} mem_cfg;

/* Variables Globales*/
t_config* config;
t_log* mem_log;
mem_cfg mem_config;

int mem_initialize();
void crear_log();
void imprimir_config();
void liberar_mem_config(mem_cfg mem_config);
void mem_exit();

void imprimir_arrays(char** split,char* nombre);
void split_liberar(char** split);
int split_cant_elem(char**split);

#endif /* CONFIG_CONFIG_H_ */

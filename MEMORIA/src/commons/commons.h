/*
 * config.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef COMMONS_COMMONS_H_
#define COMMONS_COMMONS_H_

	#include <stdio.h>
	#include <stdlib.h>
	#include <commons/config.h>
	#include <commons/log.h>
	#include <commons/collections/list.h>
	#include <string.h>
	#include <shared/utils.h>

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

	typedef struct {
		int32_t timestamp;
		u_int16_t key;
		char value[];
	} fila_TFrames;

	typedef struct {
		int32_t numero_pagina;
		fila_TFrames *frame;
		int8_t modificado;
		int32_t ultimo_uso; //ultimo tiempo de uso. Lo usa el LRU
		//TODO verificar si hay que agregar el puntero al segmento
	} fila_TPaginas;

	typedef struct {
		char nombre_tabla[255];
		t_list* paginas;
	} fila_TSegmentos;

	/* Variables Globales*/
	int socketServidor;
	int socketClienteLfs;
	int maximo_value; //TAMANIO MAXIMO DEL VALUE RECIBIDO EN BYTES POR LFS
	int cantidad_frames;
	t_list* tabla_frames;
	t_list* tabla_paginas;
	t_list* tabla_segmentos;

	t_config* config;
	t_log* mem_log;
	mem_cfg mem_config;

	int mem_initialize();
	void crear_log();
	void imprimir_config();
	void liberar_tablas();
	void liberar_mem_config(mem_cfg mem_config);
	void liberar_fila_frame(fila_TFrames* fila_frame);
	void liberar_fila_paginas(fila_TPaginas* fila_pagina);
	void liberar_tabla_frames(t_list* tabla_frames);
	void liberar_tabla_paginas(t_list* tabla_paginas);
	void liberar_tabla_segmentos(t_list* tabla_segmentos);
	void mem_exit();

	void imprimir_arrays(char** split,char* nombre);

#endif /* COMMONS_COMMONS_H_ */

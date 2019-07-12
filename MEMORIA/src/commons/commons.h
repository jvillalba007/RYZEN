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
	#include <shared/socket.h>
	#include <shared/protocolo.h>
	#include <commons/bitarray.h>
	#include <inttypes.h>

	#define pathCFG "MEMORIA.CFG"
	#define pathLOG "MEMORIA.LOG"

	typedef struct {
		char* puerto_mem;
		char* ip_mem;
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
		uint64_t timestamp;
		u_int16_t key;
		char* value;
	} fila_Frames;

	typedef struct {
		int32_t numero_pagina; //TODO: verificar si esto sirve
		char* frame_registro;
		int8_t modificado;
		uint64_t ultimo_uso; //ultimo tiempo de uso. Lo usa el LRU
		//TODO verificar si hay que agregar el puntero al segmento
	} fila_TPaginas;

	typedef struct {
		char *nombre_tabla; //TODO validar con ayudantes si se pyede definir como char[255]
		t_list* paginas;
	} fila_TSegmentos;


	typedef struct{
	    int numero_memoria;
	    char* ip;
	    char* puerto;
	    int socket;
	    bool activa;
	} t_memoria;


	/* Variables Globales*/
	int socketServidor;
	int socketClienteLfs;

	int maximo_value; //TAMANIO MAXIMO DEL VALUE RECIBIDO EN BYTES POR LFS
	int cantidad_frames;
	int frames_ocupados; //ME INDICA LA CANTIDAD DE FRAMES QUE ESTAN OCUPADOS ACTUALMENTE

	t_list* tabla_segmentos;
	char* memoria;

	t_config* config;
	t_log* mem_log;
	mem_cfg mem_config;

	char* bitMapStr;
	t_bitarray* bitmap_frames;

	t_list* tabla_memorias;

	int mem_initialize( char *fileCFG  );
	void crear_log( char* fileCFG );
	void imprimir_config();
	void liberar_tablas();
	void liberar_mem_config(mem_cfg mem_config);
	void liberar_fila_paginas(fila_TPaginas* fila_pagina);
	void liberar_memoria_contigua();
	void liberar_tabla_paginas(fila_TSegmentos *segmento);
	void liberar_tabla_segmentos(t_list* tabla_segmentos);
	void liberar_tabla_memorias(t_list* tabla_memorias );
	void liberar_fila_memoria(t_memoria* memoria_seed);
	void drop_tabla_paginas(fila_TSegmentos *segmento);
	void drop_fila_paginas(fila_TPaginas* fila_pagina);
	void enviar_insert_LFS(linea_insert* linea);
	void mem_exit_global();
	void mem_exit_simple();
	void imprimir_arrays(char** split,char* nombre);
	void list_iterate_pos(t_list* self, void(*closure)(void*,int*),int* pos);
	int escribir_en_frame(char* frame, fila_Frames registro);
	void leer_de_frame(char* frame, fila_Frames* registro);

#endif /* COMMONS_COMMONS_H_ */

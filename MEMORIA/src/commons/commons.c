/*
 * config.c
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#include "commons.h"

void verificarSocketLFS()
{
	pthread_mutex_lock(&mutex_socket);
	if(socketClienteLfs == -1)
	{
		socketClienteLfs = socket_connect_to_server(mem_config.ip_LFS,  mem_config.puerto_LFS );
	}
	pthread_mutex_unlock(&mutex_socket);
}

void retardo(){

	struct timespec ts;
	ts.tv_sec = mem_config.retardo_mem / 1000;
	ts.tv_nsec = (mem_config.retardo_mem  % 1000) * 1000000;

	log_info(mem_log, "Durmiendo por %d milisegundos" , mem_config.retardo_mem);
	nanosleep(&ts, NULL);
}

void leer_config() {
	mem_config.puerto_mem = strdup(config_get_string_value(config, "PUERTO"));
	mem_config.ip_mem = strdup(config_get_string_value(config, "IP"));
	mem_config.ip_LFS = strdup(config_get_string_value(config, "IP_FS"));
	mem_config.puerto_LFS = strdup(
			config_get_string_value(config, "PUERTO_FS"));
	mem_config.ip_SEEDS = config_get_array_value(config, "IP_SEEDS");
	mem_config.puerto_SEEDS = config_get_array_value(config, "PUERTO_SEEDS");
	mem_config.retardo_mem = config_get_int_value(config, "RETARDO_MEM");
	mem_config.retardo_fs = config_get_int_value(config, "RETARDO_FS");
	mem_config.tam_mem = config_get_int_value(config, "TAM_MEM");
	mem_config.retardo_journal = config_get_int_value(config,
			"RETARDO_JOURNAL");
	mem_config.retardo_gossiping = config_get_int_value(config,
			"RETARDO_GOSSIPING");
	mem_config.memory_number = config_get_int_value(config, "MEMORY_NUMBER");
	config_destroy(config);
}

int mem_initialize( char* archivo ) {

	fileCFG = archivo;
	rutaCFG = string_from_format("config/%s", fileCFG);

	config = config_create(rutaCFG);
	crear_log( fileCFG );

	log_info(mem_log, "cfg: %s", fileCFG);
	log_info(mem_log, "ruta cfg: %s", rutaCFG);

	if (config == NULL) {
		log_error(mem_log, "Error al leer ruta del archivo de configuracion");
		return -1;
	}
	log_info(mem_log, ".:: LISSANDRA-MEMORIA ::.");
	log_info(mem_log, ".:: Cargando configuracion ::.");

	leer_config();
	return 1;

}

void imprimir_config() {
	log_info(mem_log, ".:: Imprimiendo configuracion ::.");
	log_info(mem_log, "PUERTO MEMORIA: %s", mem_config.puerto_mem);
	log_info(mem_log, "IP MEMORIA: %s", mem_config.ip_mem);
	log_info(mem_log, "IP FLS: %s", mem_config.ip_LFS);
	log_info(mem_log, "PUERTO FLS: %s", mem_config.puerto_LFS);

	imprimir_arrays(mem_config.ip_SEEDS,"ARRAY DE IP SEEDS");
	imprimir_arrays(mem_config.puerto_SEEDS,"ARRAY DE PUERTO SEEDS");

	log_info(mem_log, "RETARDO_MEM: %d", mem_config.retardo_mem);
	log_info(mem_log, "RETARDO_FS: %d", mem_config.retardo_fs);
	log_info(mem_log, "TAM_MEM: %d", mem_config.tam_mem);
	log_info(mem_log, "RETARDO_JOURNAL: %d", mem_config.retardo_journal);
	log_info(mem_log, "RETARDO_GOSSIPING: %d", mem_config.retardo_gossiping);
	log_info(mem_log, "MEMORY_NUMBER: %d", mem_config.memory_number);
}

void crear_log( char* archivo ) {

	char* NombreArchivo = string_substring(archivo, 0 , strlen(archivo) - 4);
	char* fileLOG = string_from_format("%s.LOG", NombreArchivo);

	mem_log = log_create(fileLOG, "LISSANDRA-MEMORIA", false, LOG_LEVEL_TRACE);
	if (mem_log == NULL) {
		printf("No se pudo crear el log. Abortando ejecución\n");
		exit(EXIT_FAILURE);
	}

}

void imprimir_arrays(char** split,char* nombre)
{
	unsigned int i = 0;
	log_info(mem_log, "%s", nombre);
	for(; split[i] != NULL;i++){
		log_info(mem_log, "%d: %s",i,split[i]);
	}
}

void list_iterate_pos(t_list* self, void(*closure)(void*,int*),int* pos) {
	t_link_element *element = self->head;
	t_link_element *aux = NULL;
	*(pos) = 0;
	while (element != NULL) {
		aux = element->next;
		closure(element->data,pos);
		element = aux;
		*(pos) = *(pos)+1;
	}
}

int escribir_en_frame(char* frame, fila_Frames registro)
{
	retardo();

	int pos = 0;
	int len_value = strlen(registro.value);

	if(len_value >= maximo_value)
	{
		log_info(mem_log, "SEGMENTATION FAULT");
		return -1;
	}

	memcpy(frame, (void*) &(registro.timestamp), sizeof(uint64_t));
	pos+=sizeof(uint64_t);
	memcpy(frame+pos, (void*) &(registro.key), sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	memcpy(frame+pos, (void*) registro.value, len_value);
	frame[pos+len_value] = '\0';

	log_info(mem_log, "SE ESCRIBIO CORRECTAMENTE UN REGISTRO EN EL FRAME");

	return pos;
}

void leer_de_frame(char* frame, fila_Frames* registro)
{

	retardo();

	int pos = 0;
	memcpy((void*) &(registro->timestamp), (void*) frame, sizeof(uint64_t));
	pos+=sizeof(uint64_t);
	memcpy((void*) &(registro->key), (void*) frame+pos, sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);

	registro->value = frame+pos;

}

void liberar_tablas() {
	liberar_tabla_memorias( tabla_memorias );
	liberar_tabla_segmentos(tabla_segmentos);
}

void liberar_mem_config(mem_cfg mem_config)
{
	free(mem_config.puerto_mem);
	free(mem_config.ip_mem);
	free(mem_config.ip_LFS);
	free(mem_config.puerto_LFS);
	split_liberar(mem_config.ip_SEEDS);
	split_liberar(mem_config.puerto_SEEDS);
}

void liberar_memoria_contigua(){
	free(memoria);
	bitarray_destroy(bitmap_frames);
	free(bitMapStr);
	log_info(mem_log, "LIBERADO MEMORIA CONTIGUA");

}

void liberar_fila_paginas(fila_TPaginas* fila_pagina)
{
	free(fila_pagina);
}

void liberar_tabla_paginas(fila_TSegmentos *segmento){
	list_iterate(segmento->paginas,(void*)liberar_fila_paginas);
	list_destroy(segmento->paginas);
	log_info(mem_log, "LIBERADO TABLA DE PAGINAS");

	free( segmento->nombre_tabla );//SE AGREGO AHORA
	free(segmento);
	log_info(mem_log, "LIBERADO SEGMENTO");

}

void liberar_tabla_segmentos(t_list* tabla_segmentos) {
	list_iterate(tabla_segmentos,(void*)liberar_tabla_paginas);
	list_destroy(tabla_segmentos);
	log_info(mem_log, "LIBERADO TABLA DE SEGMENTOS");
}

void liberar_tabla_memorias(t_list* tabla_memorias) {
	list_destroy_and_destroy_elements(tabla_memorias,(void*)liberar_fila_memoria);
	log_info(mem_log, "LIBERADO TABLA DE MEMORIAS");
}

void liberar_fila_memoria(t_memoria* memoria_seed){
	free(memoria_seed->ip);
	free(memoria_seed->puerto);
	free(memoria_seed);
}

void drop_tabla_paginas(fila_TSegmentos *segmento, int tamanio_fila_Frames){

	void drop_fila_paginas(fila_TPaginas* fila_pagina)
	{
		int nro_frame = (int)(fila_pagina->frame_registro-memoria)   / tamanio_fila_Frames;
		bitarray_clean_bit(bitmap_frames, nro_frame);
		log_info(mem_log, "FRAME DISPONIBLE N°: %d",nro_frame);
		free(fila_pagina);
	}

	frames_ocupados = frames_ocupados - list_size(segmento->paginas);
	list_iterate(segmento->paginas,(void*)drop_fila_paginas);
	list_destroy(segmento->paginas);
	log_info(mem_log, "LIBERADO TABLA DE PAGINAS");
}

void enviar_insert_LFS(linea_insert* linea) {
	int size;
	char* buffer = serializar_insert(*linea, &size);
	t_header paquete;
	paquete.emisor = MEMORIA;
	paquete.tipo_mensaje = INSERT;
	paquete.payload_size = size;

	verificarSocketLFS();

	send(socketClienteLfs, &paquete, sizeof(paquete), 0);
	send(socketClienteLfs, buffer, size, 0);

	free(linea->value);
	free(buffer);
}

void mem_exit_global() {
	(socketClienteLfs == -1) ? 0 : close(socketClienteLfs);
	liberar_tablas();
	liberar_memoria_contigua();
	mem_exit_simple();
}

void mem_exit_simple() {
	liberar_mem_config(mem_config);
	log_info(mem_log, "[MEMORIA] LIBERO MEMORIA CONFIG");
	log_destroy(mem_log);
}



/*
 * protocolo.h
 *
 *  Created on: 18 may. 2019
 *      Author: utnso
 */

#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <inttypes.h>

	typedef struct {
		char* tabla;
		u_int16_t key;
		char* value;
	} linea_insert;

	typedef struct {
		char* tabla;
		u_int16_t key;
	} linea_select;

	typedef struct {
		char* value;
		uint64_t timestamp;
	} linea_response_select;

	typedef struct {
		char* tabla;
		char* tipo_consistencia;
		u_int8_t nro_particiones;
		u_int32_t tiempo_compactacion;
	} linea_create;

	typedef struct{
	    int numero_memoria;
	    char* ip;
	    char* puerto;
	    int socket;
	    bool activa;
	} pmemoria;

char* serializar_insert(linea_insert, int*);
void deserializar_insert(char*,linea_insert*);

char* serializar_select(linea_select, int*);
void deserializar_select(char*,linea_select*);

char* serializar_string(char*, int*);
char* deserializar_string(char*);

char* serializar_memorias(t_list*,int*);
t_list* deserializar_memorias(char*);
char* serializar_memoria(pmemoria, int*);
void deserializar_memoria(char*, pmemoria*);

char* serializar_describe(t_list*,int*);
t_list* deserializar_describe(char*);

char* serializar_create(linea_create, int*);
void deserializar_create(char*,linea_create*);

char* serializar_response_select(linea_response_select, int*);
void deserializar_response_select(char*,linea_response_select*);

#endif /* PROTOCOLO_H_ */

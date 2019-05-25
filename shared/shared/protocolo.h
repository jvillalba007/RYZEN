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

	typedef struct {
		char* tabla;
		u_int16_t key;
		char* value;
	} linea_insert;

	typedef struct {
		char* tabla;
		u_int16_t key;
	} linea_select;

char* serializar_insert(linea_insert, int*);
void deserializar_insert(char*,linea_insert*);

char* serializar_select(linea_select, int*);
void deserializar_select(char*,linea_select*);

char* serializar_string(char*, int*);
char* deserializar_string(char*);

#endif /* PROTOCOLO_H_ */

#include "protocolo.h"

char* serializar_insert(linea_insert insert, int* longitud)
{
	int pos = 0;
	u_int16_t len_tabla = strlen(insert.tabla);
	u_int16_t len_value = strlen(insert.value);
	int buffer_size = sizeof(u_int16_t) + len_tabla + sizeof(u_int16_t) + sizeof(u_int16_t) + len_value;

	char* buffer = malloc(buffer_size);
	memcpy(buffer, (void*) &(len_tabla), sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	memcpy(buffer+pos, (void*) insert.tabla, len_tabla);
	pos+=len_tabla;
	memcpy(buffer+pos, (void*) &(insert.key), sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	memcpy(buffer+pos, (void*) &(len_value), sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	memcpy(buffer+pos, (void*) insert.value, len_value);
	*(longitud) = buffer_size;

	return buffer;
}

void deserializar_insert(char* buffer, linea_insert* insert)
{
	int pos = 0;
	u_int16_t len_tabla;
	u_int16_t len_value;

	memcpy((void*) &(len_tabla), (void*) buffer, sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	insert->tabla = malloc(len_tabla+1);
	memcpy((void*) insert->tabla, (void*) buffer+pos,len_tabla);
	insert->tabla[len_tabla] = '\0';
	pos+=len_tabla;
	memcpy((void*) &(insert->key), (void*) buffer+pos, sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	memcpy((void*) &(len_value), (void*) buffer+pos, sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	insert->value = malloc(len_value+1);
	memcpy((void*) insert->value, (void*) buffer+pos,len_value);
	insert->value[len_value] = '\0';
}

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

char* serializar_select(linea_select linea, int* longitud)
{
	int pos = 0;
	u_int16_t len_tabla = strlen(linea.tabla);
	int buffer_size = sizeof(u_int16_t) + len_tabla + sizeof(u_int16_t);

	char* buffer = malloc(buffer_size);
	memcpy(buffer, (void*) &(len_tabla), sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	memcpy(buffer+pos, (void*) linea.tabla, len_tabla);
	pos+=len_tabla;
	memcpy(buffer+pos, (void*) &(linea.key), sizeof(u_int16_t));
	*(longitud) = buffer_size;

	return buffer;
}

void deserializar_select(char* buffer, linea_select* linea)
{
	int pos = 0;
	u_int16_t len_tabla;

	memcpy((void*) &(len_tabla), (void*) buffer, sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	linea->tabla = malloc(len_tabla+1);
	memcpy((void*) linea->tabla, (void*) buffer+pos,len_tabla);
	linea->tabla[len_tabla] = '\0';
	pos+=len_tabla;
	memcpy((void*) &(linea->key), (void*) buffer+pos, sizeof(u_int16_t));
}

char* serializar_string(char* string, int* longitud)
{
	int pos = 0;
	u_int16_t len_string = strlen(string);
	int buffer_size = sizeof(u_int16_t) + len_string;

	char* buffer = malloc(buffer_size);
	memcpy(buffer, (void*) &(len_string), sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	memcpy(buffer+pos, (void*) string, len_string);
	*(longitud) = buffer_size;

	return buffer;
}

char* deserializar_string(char* buffer)
{
	int pos = 0;
	u_int16_t len_string;

	memcpy((void*) &(len_string), (void*) buffer, sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);

	char* string = malloc(len_string+1);
	memcpy((void*) string, (void*) buffer+pos,len_string);
	string[len_string] = '\0';

	return string;
}

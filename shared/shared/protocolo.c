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

char* serializar_describe(t_list* describes,int* longitud)
{
	char* buffer = malloc(sizeof(int));
	int pos = 0;
	int buffer_size = 0;

	int cant_elementos = list_size(describes);
	memcpy(buffer, (void*) &(cant_elementos), sizeof(int));
	pos += sizeof(int);
	buffer_size += sizeof(int);

	void serializar(linea_create* linea)
	{
		int longitud;

		linea_create lineac;
		lineac.tabla = linea->tabla;
		lineac.nro_particiones = linea->nro_particiones;
		lineac.tiempo_compactacion = linea->tiempo_compactacion;
		lineac.tipo_consistencia = linea->tipo_consistencia;

		char* serializado = serializar_create(lineac,&longitud);

		buffer = realloc(buffer, buffer_size + longitud);
		memcpy(buffer+pos, (void*) serializado, longitud);
		pos += longitud;
		buffer_size += longitud;

		free(serializado);
	}

	list_iterate(describes,(void*) serializar);
	*(longitud) = buffer_size;

	return buffer;
}

t_list* deserializar_describe(char* buffer)
{

	t_list* describes = list_create();
	int cant_elementos = 0;
	int pos = 0;
	memcpy((void*) &(cant_elementos), (void*) buffer, sizeof(int));
	pos += sizeof(int);

	for(int i=0;i<cant_elementos;i++)
	{
		linea_create* linea = malloc(sizeof(linea_create));
		deserializar_create(buffer+pos, linea);
		pos += sizeof(u_int16_t) + strlen(linea->tabla) + sizeof(u_int8_t) + strlen(linea->tipo_consistencia) + sizeof(u_int8_t) + sizeof(u_int32_t);
		list_add(describes,linea);
	}

	return describes;
}

char* serializar_create(linea_create linea, int* longitud)
{
	int pos = 0;
	u_int16_t len_tabla = strlen(linea.tabla);
	u_int8_t len_tc = strlen(linea.tipo_consistencia);
	int buffer_size = sizeof(u_int16_t) + len_tabla + sizeof(u_int8_t) + len_tc + sizeof(u_int8_t) + sizeof(u_int32_t);

	char* buffer = malloc(buffer_size);
	memcpy(buffer, (void*) &(len_tabla), sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	memcpy(buffer+pos, (void*) linea.tabla, len_tabla);
	pos+=len_tabla;
	memcpy(buffer+pos, (void*) &(len_tc), sizeof(u_int8_t));
	pos+=sizeof(u_int8_t);
	memcpy(buffer+pos, (void*) linea.tipo_consistencia, len_tc);
	pos+=len_tc;
	memcpy(buffer+pos, (void*) &(linea.nro_particiones), sizeof(u_int8_t));
	pos+=sizeof(u_int8_t);
	memcpy(buffer+pos, (void*) &(linea.tiempo_compactacion), sizeof(u_int32_t));
	*(longitud) = buffer_size;

	return buffer;
}

void deserializar_create(char* buffer, linea_create* linea)
{
	int pos = 0;
	u_int16_t len_tabla;
	u_int8_t  len_tc;

	memcpy((void*) &(len_tabla), (void*) buffer, sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	linea->tabla = malloc(len_tabla+1);
	memcpy((void*) linea->tabla, (void*) buffer+pos,len_tabla);
	linea->tabla[len_tabla] = '\0';
	pos+=len_tabla;
	memcpy((void*) &(len_tc), (void*) buffer+pos, sizeof(u_int8_t));
	pos+=sizeof(u_int8_t);
	linea->tipo_consistencia = malloc(len_tc+1);
	memcpy((void*) linea->tipo_consistencia, (void*) buffer+pos,len_tc);
	linea->tipo_consistencia[len_tc] = '\0';
	pos+=len_tc;
	memcpy((void*) &(linea->nro_particiones), (void*) buffer+pos, sizeof(u_int8_t));
	pos+=sizeof(u_int8_t );
	memcpy((void*) &(linea->tiempo_compactacion), (void*) buffer+pos, sizeof(u_int32_t));
}

char* serializar_response_select(linea_response_select linea, int* longitud)
{
	int pos = 0;
	u_int16_t len_value = strlen(linea.value);
	int buffer_size = sizeof(u_int16_t) + len_value + sizeof(u_int16_t);

	char* buffer = malloc(buffer_size);
	memcpy(buffer, (void*) &(len_value), sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	memcpy(buffer+pos, (void*) linea.value, len_value);
	pos+=len_value;
	memcpy(buffer+pos, (void*) &(linea.timestamp), sizeof(uint64_t));
	*(longitud) = buffer_size;

	return buffer;
}

void deserializar_response_select(char* buffer, linea_response_select* linea)
{
	int pos = 0;
	u_int16_t len_value;

	memcpy((void*) &(len_value), (void*) buffer, sizeof(u_int16_t));
	pos+=sizeof(u_int16_t);
	linea->value = malloc(len_value+1);
	memcpy((void*) linea->value, (void*) buffer+pos,len_value);
	linea->value[len_value] = '\0';
	pos+=len_value;
	memcpy((void*) &(linea->timestamp), (void*) buffer+pos, sizeof(uint64_t));
}

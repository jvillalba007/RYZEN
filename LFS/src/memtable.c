/*
 * memtable.c
 *
 *  Created on: 15 jun. 2019
 *      Author: utnso
 */

#include "memtable.h"

//Create_memtable
fila_memtable* create_memtable(char* tabla)
{

	fila_memtable* ftabla = malloc(sizeof(fila_memtable));
	ftabla->tabla = strdup(tabla);
	ftabla->registros = NULL;


	list_add(memtable,ftabla);

	return ftabla;
}

//Obtener tabla de la memtable
fila_memtable* obtener_tabla(char *nombre_tabla){

	bool buscar_tabla(fila_memtable* fm) {
		if(string_equals_ignore_case(fm->tabla, nombre_tabla)) return true;
		return false;
	}

	fila_memtable *segmento = NULL;
	if(!list_is_empty(memtable))
	{
		segmento = list_find(memtable,(void*)buscar_tabla);
	}

	return segmento;
}

//insert_memtable
void insert_memtable(char* tabla,fila_registros* registro)
{

	pthread_mutex_lock(&mem_mutex);

	fila_memtable* ftabla = obtener_tabla(tabla);
	if( ftabla == NULL ) ftabla = create_memtable(tabla);
	if( ftabla->registros == NULL) ftabla->registros = list_create();

	list_add(ftabla->registros,registro);

	pthread_mutex_unlock(&mem_mutex);
}

//Obtener registros de tabla que esta en memtable que coincide con la key buscada
t_list* obtener_tabla_registros(fila_memtable* ftabla, u_int16_t key){

	t_list* registros = list_create();

	void buscar_registros(fila_registros* r) {
		if( r->key == key )
		{
			fila_registros* registro = malloc(sizeof(fila_registros));
			registro->timestamp = r->timestamp;
			registro->key = r->key;
			registro->value = strdup(r->value);
			list_add(registros,registro);
		}
	}



	if(!list_is_empty(ftabla->registros))
	{
		list_iterate(ftabla->registros,(void*)buscar_registros);
	}

	if(list_is_empty(registros))
	{
		list_destroy(registros);
		return NULL;
	}

	return registros;
}

//select_memtable
t_list* select_memtable(char* tabla,u_int16_t key)
{
	pthread_mutex_lock(&mem_mutex);

	fila_memtable* ftabla = obtener_tabla(tabla);
	if( ftabla == NULL ){
		log_info(g_logger, "NO SE ENCONTRO %s en memtable", tabla);
		return NULL;
	}
	t_list* registros =  obtener_tabla_registros(ftabla,key);
	if( registros == NULL ) log_info(g_logger, "NO SE ENCONTRO KEY %d en TABLA %s de memtable",key,tabla);

	pthread_mutex_unlock(&mem_mutex);

	return registros;

}

//libera el registro
void liberar_registros(fila_registros* registro)
{
	free(registro->value);
	free(registro);
}


void liberador_registros(fila_memtable* ftabla) {
	list_iterate(ftabla->registros, (void*) liberar_registros);
	list_destroy(ftabla->registros);
	log_info(g_logger, "LIBERADO REGISTROS");
}

//drop_memtable
void drop_memtable(char* tabla){

	bool buscar_tabla(fila_memtable* fm) {
		if(string_equals_ignore_case(fm->tabla, tabla)) return true;
		return false;
	}

	fila_memtable* ftabla = obtener_tabla(tabla);

	if( ftabla == NULL )
	{
		log_info(g_logger,"TABLA: %s NO EXISTE en memtable",tabla);
	}
	else
	{
		liberador_registros(ftabla);
		list_remove_by_condition(memtable,(void*)buscar_tabla);
		free(ftabla->tabla);
		free(ftabla);
		log_info(g_logger,"DROPEADO TABLA %s de MEMTABLE",tabla);
	}

}

//libera los registros y finalmente la tabla
void liberar_tablas(fila_memtable* ftabla){
	liberador_registros(ftabla);
	free(ftabla->tabla);
	free(ftabla);
	log_info(g_logger, "LIBERADO TABLA de memtable");

}

void liberar_tablas_memtable(t_list* memtable) {
	list_iterate(memtable,(void*)liberar_tablas);
	list_destroy(memtable);
	log_info(g_logger, "LIBERADO MEMTABLE");
}

//esta funcion se debe usar al final del main para liberar toda la memtable
void liberar_memtable() {
	liberar_tablas_memtable(memtable);
}


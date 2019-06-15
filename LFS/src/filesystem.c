/*
 * filesystem.c
 *
 *  Created on: 15 jun. 2019
 *      Author: utnso
 */

#include "config/config_LFS.h"

void crearParticiones(char* table_name, int partitions, int* ok){

	char* _list_to_string(t_list* list, int i){
		char* ret = string_new();
		char* aux;
		string_append(&ret, "[");
		aux = string_itoa((int) list_get(list, i));
		string_append(&ret, aux);
		free(aux);

		string_append(&ret, "]");
		return ret;
	}

	FILE *archivo;
	int blocks_size;
	sscanf(BLOCK_SIZE, "%d", &blocks_size);
	int i, bloques_necesarios = partitions;
	t_list* lista_nro_bloques = list_create();

	/* Busco en el bitmap la cantidad de bloques necesarios */
	for(i = 0; i < bitmap->size * 8 && lista_nro_bloques->elements_count < bloques_necesarios; i++){
		if(!bitarray_test_bit(bitmap, i)){ // Bloque disponible
			list_add(lista_nro_bloques, (void*) i);
			bitarray_set_bit(bitmap, i);
		}
	}

	if(lista_nro_bloques->elements_count < bloques_necesarios){ // Espacio insuficiente
		*ok = -1;
		/* Limpio del bitmap los bloques que reserve */
		for(i = 0; i < list_size(lista_nro_bloques); i++){
			bitarray_clean_bit(bitmap, (int) list_get(lista_nro_bloques, i));
		}
		list_destroy(lista_nro_bloques);
		return;
	}

	/* OK, puedo crear el archivo */
	*ok = 1;

	/* Creo las particiones */
	for(int i = 0; i < partitions; i++)
	{
		char* rutaParticiones = string_new();
		string_append(&rutaParticiones, "tables/");
		string_append(&rutaParticiones, table_name);
		string_append(&rutaParticiones, "/");
		char* nro_particion = string_itoa(i);
		string_append(&rutaParticiones, nro_particion);

		char* rutaParticionesFinal = generate_path(rutaParticiones, "", ".bin");
		archivo = fopen(rutaParticionesFinal, "wb+");
		fclose(archivo);
		t_config* config_archivo = config_create(rutaParticionesFinal);
		config_set_value(config_archivo, "TAMANIO", "0");
		char* bloques_str = _list_to_string(lista_nro_bloques,i);
		config_set_value(config_archivo, "BLOQUES", bloques_str);
		config_save(config_archivo);
		free(bloques_str);
		config_destroy(config_archivo);
		free(nro_particion);
		free(rutaParticiones);
		free(rutaParticionesFinal);

	}

	list_destroy(lista_nro_bloques);
}


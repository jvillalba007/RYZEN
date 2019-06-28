/*
 * compactator.c
 *
 *  Created on: Jun 26, 2019
 *      Author: axel
 */


#include "API.h"
#include "compactator.h"

int compactate(char* table){
	bool exist;
	exist = table_exist(table);
	if (!exist){
		// tabla ya no existe
		kill_compactator_thread(table);
		return 1;
	}

	int rename_success;
	rename_success = rename_temporal_files(table);
	if (rename_success == 1){ // no hay nada para compactar
		log_info(g_logger, "No hay nada para compactar");
		return 0;
	}else if(rename_success == 2){
		log_info(g_logger, "La compactación para la tabla %s falló. Chequear logs por errores", table);
		return 1;
	}

	t_list* new_rows;
	new_rows = get_last_rows(table);

	uint64_t blocked_time;
	uint64_t block_start = getCurrentTime();
	block_table(table);
	//----------------------------------------------------BLOCKED TABLE----------------------------------------------------
	clean_blocks(table);

	recreate_partitions(table, new_rows);

	unblock_table(table);
	//----------------------------------------------------UNBLOCKED TABLE----------------------------------------------------
	uint64_t block_finish = getCurrentTime();
	blocked_time = block_finish - block_start;

	list_destroy_and_destroy_elements(new_rows, (void*) liberar_registros);

	log_info(g_logger, "Compactación para la tabla %s tardó %" PRIu64 " milisegundos", table, blocked_time);

	return 0;
}

int kill_compactator_thread(char* table){

}

void block_table(char* table){
	// usar un diccionario GLOBAL de las commons, la key es el nombre de la tabla y el value es 1 si está bloqueada, 0 si no esta bloqueada.
	// el SELECT o cualquier otra operacion va a tener que consultar esto

}

void unblock_table(char* table){


}

void clean_blocks(char* table){
	int ok;

	char* table_path;
	table_path = generate_path(table, TABLES_FOLDER, "");

	DIR *d;
	struct dirent *dir;
	char* file_path;

	d = opendir(table_path);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0 || strcmp(dir->d_name, "Metadata") == 0)
				continue;

			file_path = strdup(table_path);
			string_append(&file_path, "/");
			string_append(&file_path, dir->d_name);
			log_info(g_logger, "Liberando bloques para %s ", file_path);

			if (string_ends_with(dir->d_name, ".tmpc")){
				borrar_archivo(file_path, &ok);
			}else{
				liberar_bloques(file_path, &ok);
				free(file_path);
			}


		 }
		closedir(d);
	}

	free(table_path);

}


void recreate_partitions(char* table, t_list* registros){

	void _compact_registro_particion(fila_registros* registro){

		char* partition_path;
		partition_path = get_partition_for_key(table, registro->key);

		int ok;
		char *data = string_from_format("%" PRIu64 ";%d;%s\n",registro->timestamp,registro->key,registro->value);
		guardarDatos(partition_path, strlen(data), data, &ok);

		free(data);
		free(partition_path);

	}

	list_iterate(registros, (void*) _compact_registro_particion);

}

t_list* get_last_rows(char* table){
	t_list* rows = list_create();

	char* table_path;
	table_path = generate_path(table, TABLES_FOLDER, "");

	DIR *d;
	struct dirent *dir;
	d = opendir(table_path);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if(string_ends_with(dir->d_name,".tmpc") || string_ends_with(dir->d_name,".bin")){

            	char * resolved_path;
            	char * filename = string_new();
            	string_append(&filename, "/");
            	string_append(&filename, dir->d_name);
            	resolved_path = generate_path(filename, table_path, "");

            	char* temps_buffer;
            	int temp_buffer_size = 0;

            	obtenerDatos(resolved_path, &temps_buffer, &temp_buffer_size);

            	if (temp_buffer_size > 0){
            		t_list* tmp_list;
					tmp_list = temp_buffer_size ? buffer_to_list_registros(temps_buffer) : NULL;

					list_add_all(rows, tmp_list);

					tmp_list ? list_destroy(tmp_list) : 0;

					free(temps_buffer);
            	}
				free(resolved_path);
            	free(filename);


			}
		}
		closedir(d);
	}


	if (list_is_empty(rows)){
		log_error(g_logger, "No se encontraron registros en los bloques de temporales y particiones durante la compactación");
		return NULL;
	}

	get_last_value_for_each_key(rows);

	free(table_path);

	return rows;
}

int rename_temporal_files(char* table){
	bool no_new_temporal_files = true;

	char* table_path;
	table_path = generate_path(table, TABLES_FOLDER, "");

	DIR *d;
	struct dirent *dir;
	d = opendir(table_path);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if(string_ends_with(dir->d_name,".tmp")){
				no_new_temporal_files = false;

				char * old_path;
				char * filename = string_new();
				string_append(&filename, "/");
				string_append(&filename, dir->d_name);
				old_path = generate_path(filename, table_path, "");

				char * new_path;
				char * filename2 = string_new();
				string_append(&filename2, "/");
				string_append(&filename2, dir->d_name);
				string_append(&filename2, "c"); // to become .tmpc
				new_path = generate_path(filename2, table_path, "");

				int ret;
				ret = rename(old_path, new_path);

				free(old_path);
				free(new_path);
				free(filename);
				free(filename2);

				if(ret == 0) {
				  log_info(g_logger, "File renamed successfully");
				} else {
				  log_error(g_logger, "Error: unable to rename the file");
				  free(table_path);
				  return 2;
				}

			}
		}
		closedir(d);
	}

	free(table_path);

	if (no_new_temporal_files){ // no hay nada para compactar
		return 1;
	}

	return 0;
}

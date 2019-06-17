/*
 * filesystem.h
 *
 *  Created on: 15 jun. 2019
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
	#define FILESYSTEM_H_

	#define TABLES_FOLDER "tables/"

	void crearParticiones(char* table_name, int partitions, int* ok);
	void obtenerDatos(char* pathParticion, void** ret_buffer, int* ret_buffer_size);
	void guardarDatos(char* pathParticion, int bytes, void* buffer, int* ok);
	char* get_partition_for_key(char* table_name, char* key);
	void borrar_archivo(char* path, int* ok);

#endif /* FILESYSTEM_H_ */

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
	void guardarDatos(char* pathParticion, int bytes, void* buffer, int* ok);
	char* get_partition_for_key(char* table_name, char* key);

#endif /* FILESYSTEM_H_ */

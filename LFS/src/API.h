#ifndef API_H_
	#define API_H_

		#include <stdio.h>
		#include <string.h>
		#include <stdlib.h>
		#include <unistd.h>
		#include <dirent.h>
		#include <sys/types.h>
		#include <sys/stat.h>
		#include <commons/string.h>
		#include <errno.h>
		#include <time.h>
		#include <inttypes.h>

		#include <shared/utils.h>
		#include "shared/protocolo.h"
		#include "config/config_LFS.h"
		#include "filesystem.h"
		#include "memtable.h"


		#define TABLES_FOLDER "tables/"

		void consola_procesar_comando(char*);
		void procesar_insert(int, char**);
		char* extract_value_from_key(char*, char*);
		void procesar_select(char**);
		void procesar_drop(char**);
		void* procesar_describe(int, char**);
		linea_create* read_table_metadata(char*);
		void liberar_linea_create(linea_create*);
		int drop_table(char*);
		int insert_record(linea_insert*, char*);
		void liberar_linea_insert(linea_insert*);
		void procesar_create(char**);
		int create_table(char*, char*, char*, char*);
		char* select_table_key(linea_select*);
		void liberar_linea_select(linea_select*);
		void retardo(void);

		const char* folder_path;

#endif /* API_H_ */

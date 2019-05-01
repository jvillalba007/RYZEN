#ifndef API_H_
	#define API_H_

		#include <stdio.h>
		#include <string.h>
		#include <stdlib.h>
		#include <commons/string.h>
		#include <errno.h>

		#include "shared/utils.h"

		#define TABLES_FOLDER "tables/"

		void consola_procesar_comando(char*);
		void procesar_insert(int, char** , char*);

		const char* folder_path;

#endif /* API_H_ */

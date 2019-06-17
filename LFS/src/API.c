#include "API.h"

int file_write_key_value (char* full_path, char* key, char* value, char* timestamp){

	FILE * fPtr;
	fPtr = fopen(full_path, "a");

	if(fPtr == NULL)
	{
		printf("Unable to create file: %s \n %s \n", full_path, (char *) strerror(errno));
		return 1;
	}

	char* key_value = (char*) calloc(strlen(timestamp) + 1 + strlen(value) + 1 + strlen(key) + 1, sizeof(char));

	// Concatener linea a colocar
	strcat(key_value, timestamp);
	strcat(key_value, ";");
	strcat(key_value, key);
	strcat(key_value, ";");
	strcat(key_value, value);

	fputs(key_value, fPtr);
	fputs("\n", fPtr);

	fclose(fPtr);


	log_info(g_logger, "Insertado %s en %s", key_value, full_path);
    free(key_value);

    return 0;
}

void procesar_insert(int cant_parametros, char** parametros_no_value, char* value){

	char* table_name = parametros_no_value[1];
	string_to_upper(table_name);
	char* full_path;
	full_path = generate_path(table_name, TABLES_FOLDER, ".txt");

	if( access( full_path, F_OK ) == -1 ) {
	    // file doesn't exist
		free(full_path);
		printf("No existe la tabla especificada. Creala.");
		return;
	}

	char* key = parametros_no_value[2];
	char* timestamp;

	if (cant_parametros == 3){
		// Timestamp no proporcionado
		timestamp = (char*)calloc(sizeof(int32_t) + 1, sizeof(char));
		time_t current_time;
		current_time = (int32_t) time(NULL);
		sprintf(timestamp, "%lu", current_time );
		file_write_key_value(full_path, key, value, timestamp);

		free(timestamp);

	} else if (cant_parametros == 4) {
		//Timestamp proporcionado
		timestamp = parametros_no_value[3];
		file_write_key_value(full_path, key, value, timestamp);

	}

	free(full_path);

}

int procesar_drop(char** parametros){
	char* table_name;
	table_name = parametros[1];
	string_to_upper(table_name);

	struct stat st = {0};
	char* table_path;
	table_path = generate_path(table_name, TABLES_FOLDER, "");

	int* ok;

	if (stat(table_path, &st) == -1) {
		printf("La tabla especificada no existe. \n");
		log_error(g_logger, "La tabla %s no existe", table_name);
		free(table_path);

		return 1;
	}else{
		// Already exists
		drop_memtable(table_name);

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
	            log_info(g_logger, "Accedí a %s ", file_path);
	            borrar_archivo(file_path, &ok);

	         }
	        closedir(d);
	    }

	    char* metadata_path;
		metadata_path = generate_path("/Metadata", table_path, "");
		remove(metadata_path);
		free(metadata_path);

		rmdir(table_path);

	    free(table_path);

		return 0;
	}

}


void procesar_create(char** parametros){

	char* table_name = parametros[1];
	string_to_upper(table_name);
	char* consistency = parametros[2];
	string_to_upper(consistency);
	char* partitions = parametros[3];
	char* compact_time = parametros[4];

	if( !isNumeric(partitions) ){
		printf("Parametro particiones no es numérico. \n");
		return;
	}

	if( !isNumeric(compact_time) ){
		printf("Parametro tiempo de compactación no es numérico. \n");
		return;
	}

	// Does the table exist?
	struct stat st = {0};
	char* table_path;
	table_path = generate_path(table_name, TABLES_FOLDER, "");

	if (stat(table_path, &st) == -1) {
		// Create it
	    mkdir(table_path, 0700);
	}else{
		// Already exists
		free(table_path);
		printf("La tabla especificada ya existe. \n");
		log_info(g_logger, "La tabla %s ya existe", table_name);
		return;
	}

	// Create Metadata file
	char* metadata_path;
	metadata_path = generate_path("/Metadata", table_path, "");

	FILE * fPtr;
	fPtr = fopen(metadata_path, "a");

	if(fPtr == NULL)
	{
		printf("Unable to create file: %s \n %s \n", metadata_path, (char *) strerror(errno));
		return;
	}

	char* consist = (char*) calloc(strlen("CONSISTENCY=") + strlen(consistency) +1, sizeof(char));
	char* parts = (char*) calloc(strlen("PARTITIONS=") + strlen(partitions) +1, sizeof(char));
	char* compact = (char*) calloc(strlen("COMPACTION_TIME=") + strlen(compact_time) +1, sizeof(char));

	strcat(consist, "CONSISTENCY=");
	strcat(consist, consistency);
	strcat(parts, "PARTITIONS=");
	strcat(parts, partitions);
	strcat(compact, "COMPACTION_TIME=");
	strcat(compact, compact_time);

	fputs(consist, fPtr);
	fputs("\n", fPtr);
	fputs(parts, fPtr);
	fputs("\n", fPtr);
	fputs(compact, fPtr);
	fputs("\n", fPtr);

	int ok;
	crearParticiones(table_name,atoi(partitions),&ok);
	log_info(g_logger, "Resultado del Create %d", ok);

	// Free everything
	fclose(fPtr);
	free(metadata_path);
	free(table_path);
	free(consist);
	free(parts);
	free(compact);
}

linea_create* read_table_metadata(char* table_name){

	string_to_upper(table_name);

	// Does the table exist?
	struct stat st = {0};
	char* table_path;
	table_path = generate_path(table_name, TABLES_FOLDER, "");

	if( access( table_path, F_OK ) == -1 ) {
	    // file doesn't exist
		free(table_path);
		log_error(g_logger, "No existe la tabla especificada.");
		return NULL;
	}

	char* metadata_path;
	metadata_path = generate_path("/Metadata", table_path, "");

	t_config* metadata_t;
	metadata_t = config_create(metadata_path);

    free(metadata_path);

    char* consistency;
    u_int8_t partitions;
    u_int32_t compaction_time;

	consistency = strdup(config_get_string_value(metadata_t, "CONSISTENCY"));
	partitions = config_get_int_value(metadata_t, "PARTITIONS");
	compaction_time = config_get_int_value(metadata_t, "COMPACTION_TIME");

	linea_create* metadata_s = malloc(sizeof(linea_create));
	metadata_s->tabla = strdup(table_name);
	metadata_s->tipo_consistencia = strdup(consistency);
	metadata_s->nro_particiones = partitions;
	metadata_s->tiempo_compactacion = compaction_time;

	printf("Tabla: %s\n", table_name);
	printf("Consistencia: %s\n", consistency);
	printf("Particiones: %d\n", partitions);
	printf("Tiempo de compactación: %d\n", compaction_time);

	free(consistency);
	free(table_path);
	config_destroy(metadata_t);

    return metadata_s;
}

void* procesar_describe(int cant_parametros, char** parametros){


	// DESCRIBE TABLA
	if (cant_parametros == 2){
		char* table_name = parametros[1];
		linea_create* metadata;
		metadata = read_table_metadata(table_name);

		return metadata;
	}

	t_list * list_metadata = list_create();

    DIR *d;
    struct dirent *dir;
    d = opendir("tables");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                continue;

            list_add(list_metadata, read_table_metadata(dir->d_name));

        }
        closedir(d);
    }

    return list_metadata;
}

void liberar_metadata_struct (linea_create* metadata){
	free(metadata->tabla);
	free(metadata->tipo_consistencia);
	free(metadata);
}

void consola_procesar_comando(char* linea)
{

	char** parametros = string_split(linea, " ");
	int cant_parametros = split_cant_elem(parametros);

	if(string_equals_ignore_case(parametros[0],"INSERT")){

		char* value;
		value = string_extract_substring(linea, "\"", "\"");

		if (value == NULL) {
			puts("API Error: Valor no proporcionado\n");
			free(value);
			split_liberar(parametros);
			return ;
		}

		remove_value (linea, value); // Queda la linea sin value, solo comillas

		char** parametros_no_value = string_split(linea, " ");

		int cant_sin_value = split_cant_elem(parametros_no_value);

		if (cant_sin_value >= 3 && cant_sin_value < 5) {

			procesar_insert(cant_sin_value, parametros_no_value, value);
			free(value);
			split_liberar(parametros_no_value);

		}else{
			printf("API Error: 3 o 4 argumentos son requeridos\n");
		}
	}

	else if(string_equals_ignore_case(parametros[0],"SELECT")){
		if (cant_parametros == 3) {
			string_iterate_lines(parametros,puts);
		} else {
			printf("API Error: 2 argumentos son requeridos.\n");
		}

	}

	else if(string_equals_ignore_case(parametros[0],"CREATE")){
		if (cant_parametros == 5) {

			procesar_create(parametros);

		} else {
			printf("API Error: 4 argumentos son requeridos.\n");
		}

	}

	else if(string_equals_ignore_case(parametros[0],"DESCRIBE")){
		if (cant_parametros >= 1 && cant_parametros < 3) {
			void* response;

			response = procesar_describe(cant_parametros, parametros);
			list_destroy_and_destroy_elements(response, liberar_metadata_struct);
			free(response);

		} else {
			printf("API Error: ninguno o 1 argumento es requerido.\n");
		}

	}

	else if(string_equals_ignore_case(parametros[0],"DROP")){
		if (cant_parametros == 2) {
			int response;

			response = procesar_drop(parametros);

		} else {
			printf("API Error: 1 argumento es requerido.\n");
		}

	}

	/* Comando clear (limpiar pantalla consola) */
	else if(cant_parametros == 1 && string_equals_ignore_case(parametros[0],"CLEAR")){
		system("clear");
	}

	/* Error al ingresar comando */
	else{
		printf("No se pudo reconocer el comando\n");

	}

	split_liberar(parametros);
}


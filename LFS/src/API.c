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

	// Create partitions
	int partitions_i = 0;
	sscanf(partitions, "%d", &partitions_i); // castear a int
	char* partition_path;
	for (int i = 0; i < partitions_i; ++i) {
		char* n = string_itoa(i);
		char* path_to_n = (char*) calloc(strlen(n) + 1 + 1, sizeof(char));
		strcat(path_to_n, "/");
		strcat(path_to_n, n);
		partition_path = generate_path(path_to_n, table_path, ".bin");

		FILE * f;
		f = fopen(partition_path, "a");
		fclose(f);
		free(partition_path);
		free(path_to_n);
		free(n);

	}


	// Free everything
	fclose(fPtr);
	free(metadata_path);
	free(table_path);
	free(consist);
	free(parts);
	free(compact);
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
			string_iterate_lines(parametros,puts);
		} else {
			printf("API Error: ninguno o 1 argumento es requerido.\n");
		}

	}

	else if(string_equals_ignore_case(parametros[0],"DROP")){
		if (cant_parametros == 2) {
			string_iterate_lines(parametros,puts);
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


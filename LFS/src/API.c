#include "API.h"


int insert_record(linea_insert* datos, char* fixed_timestamp){
	string_to_upper(datos->tabla);

	char* full_path;
	full_path = generate_path(datos->tabla, TABLES_FOLDER, "");

	if( access( full_path, F_OK ) == -1 ) {
	    // file doesn't exist
		free(full_path);
		log_error(g_logger, "La tabla %s no existe", datos->tabla);
		return 1;
	}

	fila_registros* registro = malloc(sizeof(fila_registros));

	if (fixed_timestamp == NULL){
		// Timestamp no proporcionado
		registro->timestamp = getCurrentTime();
		registro->key = datos->key;
		registro->value = strdup(datos->value);
		insert_memtable(datos->tabla,registro);

	} else {
		//Timestamp proporcionado
		uint64_t timestamp;
		sscanf(fixed_timestamp, "%" PRIu64, &timestamp);

		registro->timestamp = timestamp;
		registro->key = datos->key;
		registro->value = strdup(datos->value);
		insert_memtable(datos->tabla, registro);

	}

	free(full_path);

	return 0;
}

void procesar_insert(int cant_parametros, char** parametros){

	char* table_name = parametros[1];
	char* key = parametros[2];
	char* value = parametros[3];
	char* value1 = string_substring_from(value, 1); // remove first "
	char* value2 = string_substring_until(value1, strlen(value1) - 1); //remove last "

	value = strdup(value2);
	free(value1);
	free(value2);

	linea_insert* datos = malloc(sizeof(linea_insert));
	datos->key = atoi(key);
	datos->tabla = strdup(table_name);
	datos->value = strdup(value);

	int response;
	if (cant_parametros == 4){
		response = insert_record(datos, NULL);
	}else{
		response = insert_record(datos, parametros[4]);
	}

	liberar_linea_insert(datos);

	if (response==1)
		printf("No existe la tabla especificada. \n");

	free(value);

}

int drop_table(char* table_name){

	string_to_upper(table_name);

	struct stat st = {0};
	char* table_path;
	table_path = generate_path(table_name, TABLES_FOLDER, "");

	if (stat(table_path, &st) == -1) { // table doesn't exist
		log_error(g_logger, "La tabla %s no existe", table_name);
		free(table_path);

		return 1;
	}

	int ok;

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

void procesar_drop(char** parametros){
	char* table_name;
	table_name = parametros[1];

	int response;
	response = drop_table(table_name);

	if (response == 1)
		printf("La tabla especificada no existe. \n");

}

int create_table(char* table_name, char* consistency, char* partitions, char* compact_time){

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
		return 1;
	}

	// Create Metadata file
	char* metadata_path;
	metadata_path = generate_path("/Metadata", table_path, "");

	FILE * fPtr;
	fPtr = fopen(metadata_path, "a");

	if(fPtr == NULL)
	{
		printf("Unable to create file: %s \n %s \n", metadata_path, (char *) strerror(errno));
		free(table_path);
		free(metadata_path);
		return 1;
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
	crearParticiones(table_name, atoi(partitions), &ok);


	if (ok == -1){
		log_error(g_logger, "Espacio Insuficiente");
		printf("Espacio Insuficiente\n");
		drop_table(table_name);

		fclose(fPtr);
		free(metadata_path);
		free(table_path);
		free(consist);
		free(parts);
		free(compact);

		return 1;
	}
	log_info(g_logger, "Resultado del Create %d", ok);

	// Free everything
	fclose(fPtr);
	free(metadata_path);
	free(table_path);
	free(consist);
	free(parts);
	free(compact);

	return 0;
}

void procesar_create(char** parametros){

	char* table_name = parametros[1];
	string_to_upper(table_name);
	char* consistency = parametros[2];
	string_to_upper(consistency);
	char* partitions = parametros[3];
	char* compact_time = parametros[4];

	if( !isNumeric(partitions) ){
		printf("Parametro particiones no es numérico.\n");
		return;
	}

	if( !isNumeric(compact_time) ){
		printf("Parametro tiempo de compactación no es numérico.\n");
		return;
	}

	int response;
	response = create_table(table_name, consistency, partitions, compact_time);

	if (response == 1)
		printf("Operación create no pudo ser realizada. Revise logs.\n");

}

char* select_table_key(linea_select* datos){
	string_to_upper(datos->tabla);


	char* table_path;
	table_path = generate_path(datos->tabla, TABLES_FOLDER, "");

	if( access( table_path, F_OK ) == -1 ) {
	    // file doesn't exist
		free(table_path);
		log_error(g_logger, "La tabla %s no existe", datos->tabla);
		printf("No existe la tabla especificada. \n");
		return NULL;
	}

	// get registros memtable
	t_list* select_mem;
	select_mem = select_memtable(datos->tabla, datos->key);

	// get registros particiones
	char* partition_path;
	partition_path = get_partition_for_key(datos->tabla, (u_int16_t) datos->key);

	char* registros_buffer;
	int buffer_size = 0;

	obtenerDatos(partition_path, &registros_buffer, &buffer_size);

	t_list* select_temp = list_create();

	DIR *d;
    struct dirent *dir;
    d = opendir(table_path);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if(string_ends_with(dir->d_name,".tmp") || string_ends_with(dir->d_name,".tmpc")){

            	char * resolved_path;
            	char * filename = string_new();
            	string_append(&filename, "/");
            	string_append(&filename, dir->d_name);
            	resolved_path = generate_path(filename, table_path, "");

            	char* temps_buffer;
            	int temp_buffer_size = 0;

            	obtenerDatos(resolved_path, &temps_buffer, &temp_buffer_size);

            	t_list* tmp_list;
            	tmp_list = temp_buffer_size ? buffer_to_list_registros(temps_buffer) : NULL;

            	list_add_all(select_temp, tmp_list);

            	tmp_list ? list_destroy(tmp_list) : 0;
            	free(resolved_path);
            	free(filename);
            	temp_buffer_size ? free(temps_buffer) : 0;
            }
        }
        closedir(d);
    }

	t_list* select_fs;
	select_fs = buffer_size ? buffer_to_list_registros(registros_buffer) : NULL;

	t_list* all_list = list_create();
	t_list* filtered_list;


	if (select_mem == NULL && select_fs == NULL && select_temp == NULL){ // ninguna tiene datos

		log_info(g_logger, "No existe la clave %d en la tabla %s", datos->key, datos->tabla);
		free(table_path);
		free(partition_path);
		return NULL;

	} else {


		select_mem? list_add_all(all_list, select_mem) : 0;
		select_fs? list_add_all(all_list, select_fs) : 0;
		select_temp? list_add_all(all_list, select_temp) : 0;

		filtered_list = filter_registro_list_by_key(all_list, datos->key);

	}

	if (filtered_list == NULL){
		log_info(g_logger, "No existe la clave %d en la tabla %s", datos->key, datos->tabla);
		return NULL;
	}


	char* last_value;
	last_value = get_last_value(filtered_list);

	select_mem ? list_destroy(select_mem) : 0;
	select_fs ? list_destroy(select_fs) : 0;
	select_temp ? list_destroy(select_temp) : 0;
	filtered_list ? list_destroy(filtered_list) : 0;
	all_list ? list_destroy_and_destroy_elements(all_list, (void*) liberar_registros) : 0;


	buffer_size ? free(registros_buffer) : 0;
 	free(table_path);
 	free(partition_path);

 	return last_value;

}

void procesar_select(char** parametros){

	char* table_name = parametros[1];
	char* key = parametros[2];

	linea_select* datos = malloc(sizeof(linea_select));
	datos->key = atoi(key);
	datos->tabla = strdup(table_name);

	char* response;
	response = select_table_key(datos);

	if (response != NULL){
		printf("El valor de la última clave %s es %s \n", parametros[2], response);
		free(response);
	}else{
		printf("No se encuentra la clave %s \n", parametros[2]);
	}

	liberar_linea_select(datos);

}

linea_create* read_table_metadata(char* table_name){

	string_to_upper(table_name);

	// Does the table exist?
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

void liberar_linea_create (linea_create* metadata){
	free(metadata->tabla);
	free(metadata->tipo_consistencia);
	free(metadata);
}

void liberar_linea_insert(linea_insert* metadata){
	free(metadata->tabla);
	free(metadata->value);
	free(metadata);
}

void liberar_linea_select(linea_select* metadata){
	free(metadata->tabla);
	free(metadata);
}

void consola_procesar_comando(char* linea)
{

	char** parametros = string_split(linea, " ");
	int cant_parametros = split_cant_elem(parametros);

	if(string_equals_ignore_case(parametros[0],"INSERT")){

		if (cant_parametros >= 4 && cant_parametros < 6) {

			procesar_insert(cant_parametros, parametros);

		}else{
			printf("API Error: 3 o 4 argumentos son requeridos\n");
		}
	}

	else if(string_equals_ignore_case(parametros[0],"SELECT")){
		if (cant_parametros == 3) {

			procesar_select(parametros);

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

			if (cant_parametros == 1){
				list_destroy_and_destroy_elements(response, (void*) liberar_linea_create);
			}else{
				liberar_linea_create(response);
			}


		} else {
			printf("API Error: ninguno o 1 argumento es requerido.\n");
		}

	}

	else if(string_equals_ignore_case(parametros[0],"DROP")){
		if (cant_parametros == 2) {
			procesar_drop(parametros);

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


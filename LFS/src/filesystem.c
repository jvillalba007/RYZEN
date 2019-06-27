/*
 * filesystem.c
 *
 *  Created on: 15 jun. 2019
 *      Author: utnso
 */

#include "filesystem.h"

bool does_table_exist(char* table){ //TODO: aplicar a todas las funciones que usen esto
	bool exists = true;

	char* table_path;
	table_path = generate_path(table, TABLES_FOLDER, "");

	if( access( table_path, F_OK ) == -1 ) {
	    // file doesn't exist
		free(table_path);
		log_error(g_logger, "Does table %s exist? %s", table, exists ? "true" : "false");
		exists = false;
	}

	return exists;
}


void get_last_value_for_each_key(t_list* registros){
	t_list* filtered_list = list_create();
	fila_registros* last_registro;

	bool _matches_key_and_is_not_registro(fila_registros* registro){
		if(registro->key == last_registro->key && registro != last_registro){ // si el regisro tiene la misma key pero NO ES EL ULTIMO -> BORRALO
			return true;
		}else{
			return false;
		}
	}

	int l_size = list_size(registros);

	// Recorrer toda la lista: por cada elemento, crear otra lista con los registros filtrada por key del elemento.
	// De esa lista, get el registro con el último timestamp.
	// Ahora remove de la lista principal todos los elementos que no sean ese registro pero compartan la key, es decir: si tiene la misma key pero NO ES EL ULTIMO.
	unsigned int i = 0;
	for(; i < l_size; i++){
		fila_registros* tmp_registro;
		tmp_registro = list_get(registros, i);
		u_int16_t key;
		key = tmp_registro->key;

		t_list* one_key_list;
		one_key_list = filter_registro_list_by_key(registros, key);
		fila_registros* last_registro;
		last_registro = get_last_registro(one_key_list);

		list_remove_and_destroy_by_condition(registros, _matches_key_and_is_not_registro, liberar_registros);

		list_destroy(one_key_list);
	}

}

fila_registros* get_last_registro(t_list* registros){
	fila_registros* last_registro;
	uint64_t max_timestamp = 0;

	void _update_max_timestamp(fila_registros* registro){
		if (registro->timestamp > max_timestamp){
			max_timestamp = registro->timestamp;
			last_registro = registro;
		}
	}

	list_iterate(registros, (void*) _update_max_timestamp);

	return last_registro;
}

char* get_last_value(t_list* registros){ //TODO: modificar para que use la función get_last_registro

	char* last_value = 0;
	uint64_t max_timestamp = 0;

	void _update_max_timestamp(fila_registros* registro){
		if (registro->timestamp > max_timestamp){
			max_timestamp = registro->timestamp;
			if (last_value != 0){
				free(last_value);
			}
			last_value = strdup(registro->value);
		}
	}

	list_iterate(registros, (void*) _update_max_timestamp);

	return last_value;
}

t_list* filter_registro_list_by_key(t_list* list, u_int16_t key){ //TODO: cambiar a int equals en vez de string compare

	char* key_c;
	key_c = string_itoa(key);

	bool _filter_key(fila_registros* registro){
		char* strkey = string_itoa(registro->key);
		if ( strcmp(strkey, key_c) == 0 ){
			free(strkey);
			return true;
		}else{
			free(strkey);
			return false;
		}
	}


	t_list* filtered_list;
	filtered_list = list_filter(list, (void*)_filter_key);

	if(list_is_empty(filtered_list))
	{
		free(key_c);
		list_destroy(filtered_list);
		return NULL;
	}

	free(key_c);
	return filtered_list;
}

t_list* buffer_to_list_registros(char* buffer){


	t_list* registros = list_create();
	char* strbuffer = strdup(buffer);
	char * line = strtok(strbuffer, "\n");

	while(line) {
		fila_registros* registro = malloc(sizeof(fila_registros));
		char** partes = string_split(line, ";");
		int cant = split_cant_elem(partes);

		uint64_t timestamp;
		sscanf(partes[0], "%" PRIu64, &timestamp);

		registro->timestamp = cant == 3 ? (uint64_t) timestamp : 0;
		registro->key = cant == 3 ? atoi(partes[1]) : -1;
		registro->value = cant == 3 ? strdup(partes[2]) : strdup("");


		list_add(registros, registro);

		split_liberar(partes);
		line  = strtok(NULL, "\n");
	}

	if(list_is_empty(registros))
	{
		list_destroy(registros);
		return NULL;
	}

	free(strbuffer);

	return registros;
}


void borrar_archivo(char* path, int* ok){

	void _borrar_bloque(char* nro_bloque){
		bitarray_clean_bit(bitmap, atoi(nro_bloque));
	}

	*ok = 1;

	t_config* config_archivo = config_create(path);


	char** bloques_strings = config_get_array_value(config_archivo, "BLOQUES");
	string_iterate_lines(bloques_strings, _borrar_bloque);

	remove(path);
	free(path);

	config_destroy(config_archivo);
	split_liberar(bloques_strings);
}


char* get_partition_for_key(char* table_name, u_int16_t key){


	string_to_upper(table_name);
	char* table_path;
	table_path = generate_path(table_name, TABLES_FOLDER, "");

	char* metadata_path;
	metadata_path = generate_path("/Metadata", table_path, "");

	t_config* metadata;
    metadata = config_create(metadata_path);

    free(metadata_path);

    int partitions;
	partitions = config_get_int_value(metadata, "PARTITIONS");

	config_destroy(metadata);

	int partition_assigned;
	partition_assigned = key % partitions;

	char* partition_c;
	partition_c = string_itoa(partition_assigned);

	char* partition_path;

	char* partition_p = (char*) calloc(1 + strlen(partition_c) + 1, sizeof(char));
	strcat(partition_p, "/");
	strcat(partition_p, partition_c);

	partition_path = generate_path(partition_p, table_path, ".bin");

    free(partition_p);
    free(partition_c);
    free(table_path);

	return partition_path;

}

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
	for(i = 0; i < atoi(BLOCKS) && lista_nro_bloques->elements_count < bloques_necesarios; i++){
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

void crearTemporal(char* table_name,char* temporal, int* ok){

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
	int i, bloques_necesarios = 1;
	t_list* lista_nro_bloques = list_create();

	/* Busco en el bitmap la cantidad de bloques necesarios */
	for(i = 0; i < atoi(BLOCKS) && lista_nro_bloques->elements_count < bloques_necesarios; i++){
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

		/* Creo el temporla */
		char* rutaTemporal = string_new();
		string_append(&rutaTemporal, "tables/");
		string_append(&rutaTemporal, table_name);
		string_append(&rutaTemporal, "/");
		string_append(&rutaTemporal, temporal);

		char* rutaTemporalFinal = generate_path(rutaTemporal, "", "");
		archivo = fopen(rutaTemporalFinal, "wb+");
		fclose(archivo);
		t_config* config_archivo = config_create(rutaTemporalFinal);
		config_set_value(config_archivo, "TAMANIO", "0");
		char* bloques_str = _list_to_string(lista_nro_bloques,0);
		config_set_value(config_archivo, "BLOQUES", bloques_str);
		config_save(config_archivo);
		free(bloques_str);
		config_destroy(config_archivo);
		free(rutaTemporal);
		free(rutaTemporalFinal);

	list_destroy(lista_nro_bloques);
}

void obtenerDatos(char* pathParticion, char** ret_buffer, int* ret_buffer_size){
	t_config* config_archivo;
	config_archivo = config_create(pathParticion);

	int bytes = config_get_int_value(config_archivo, "TAMANIO");

	if (bytes == 0)
	{
		log_error(g_logger, "NO HAY DATOS");
		config_destroy(config_archivo);
		return;
	}

	int offset = 0;
	*ret_buffer = calloc(bytes+1,sizeof(char));
	*ret_buffer_size = 0;
	int i = 0, indice_bloque_inicial;
	char** bloques_strings;


	// Retorna si el archivo se ha terminado
	bool _agregar_bloque_a_buffer(char* nro_bloque, int offset, int indice_bloque){
		bool ret_eof = false;

		char* ruta_bloque = string_new();
		string_append(&ruta_bloque, "bloques/");
		string_append(&ruta_bloque, nro_bloque);
		char* ruta_bloqueFinal = generate_path(ruta_bloque, "", ".bin");
		FILE* bloque = fopen(ruta_bloqueFinal, "rb");
		fseek(bloque, offset, SEEK_SET);
		free(ruta_bloque);
		free(ruta_bloqueFinal);

		int cant_bytes_a_leer = min(atoi(BLOCK_SIZE) - offset, bytes);
		/* Me fijo si es el ultimo bloque */
		if(split_cant_elem(bloques_strings) - 1 == indice_bloque){
			int tam_ultimo_bloque = config_get_int_value(config_archivo, "TAMANIO") -
					((indice_bloque_inicial + i) * atoi(BLOCK_SIZE));
			ret_eof = tam_ultimo_bloque - offset <= cant_bytes_a_leer; // Llegue al EOF
			cant_bytes_a_leer = min(tam_ultimo_bloque - offset, cant_bytes_a_leer);
		}

		char* data = calloc(cant_bytes_a_leer,sizeof(char));
		int bytes_leidos = fread(data, 1, cant_bytes_a_leer, bloque);
		memcpy((char*) (*ret_buffer) + (*ret_buffer_size), data, bytes_leidos);
		*ret_buffer_size += bytes_leidos;
		free(data);
		fclose(bloque);
		return ret_eof;
	}

	bloques_strings = config_get_array_value(config_archivo, "BLOQUES");
	// Agrego el bloque inicial:
	indice_bloque_inicial = offset/atoi(BLOCK_SIZE);
	int offset_bloque_inicial = offset - (indice_bloque_inicial * atoi(BLOCK_SIZE));
	bool eof = _agregar_bloque_a_buffer(bloques_strings[indice_bloque_inicial],  offset_bloque_inicial, indice_bloque_inicial);
	if(eof) // No tengo que ir a leer mas bloques
		bytes = 0;
	else
		bytes -= (atoi(BLOCK_SIZE) - offset_bloque_inicial);

	// Agrego el resto de bloques a abrir
	while(bytes > 0 && !eof){
		int indice = indice_bloque_inicial + ++i;
		log_info(g_logger, "AGREGO BLOQUE INDICE: %d, NRO: %s | BYTES_RESTANTES: %d", indice, bloques_strings[indice], bytes);
		eof = _agregar_bloque_a_buffer(bloques_strings[indice], 0, indice);
		bytes -= atoi(BLOCK_SIZE);
	}

	config_destroy(config_archivo);
	split_liberar(bloques_strings);
}

void guardarDatos(char* pathParticion, int bytes, void* buffer, int* ok){
	char* bloques_string;
	int offset_buffer = 0;
	int tamanio_total;
	int nuevo_indice_bloque = 0;

	void _escribir_bloque(char* nro_bloque, int offset){
		char* ruta_bloque = string_new();
		string_append(&ruta_bloque, "bloques/");
		string_append(&ruta_bloque, nro_bloque);
		char* ruta_bloqueFinal = generate_path(ruta_bloque, "", ".bin");
		FILE* bloque = fopen(ruta_bloqueFinal, "rb+");
		fseek(bloque, offset, SEEK_SET);
		free(ruta_bloque);
		free(ruta_bloqueFinal);

		int cant_bytes_a_escribir = min(atoi(BLOCK_SIZE) - offset, bytes);
		tamanio_total = max(tamanio_total, offset + cant_bytes_a_escribir + (nuevo_indice_bloque * atoi(BLOCK_SIZE)));
		fwrite(buffer + offset_buffer, 1, cant_bytes_a_escribir, bloque);
		offset_buffer += cant_bytes_a_escribir;

		fclose(bloque);
	}

	char* _obtener_bloque(){
		char* nro_bloque_str;
		int nro_bloque;
		bool bloque_disponible;

		for(nro_bloque = 0;
			nro_bloque < atoi(BLOCKS) && !(bloque_disponible = !(bitarray_test_bit(bitmap, nro_bloque)));
			nro_bloque++);

		if(!bloque_disponible){
			*ok = -1;
			return NULL;
		}
		bitarray_set_bit(bitmap, nro_bloque);
		nro_bloque_str = string_itoa(nro_bloque);

		bloques_string[strlen(bloques_string) - 1] = '\0';
		string_append(&bloques_string, ",");
		string_append(&bloques_string, nro_bloque_str);
		string_append(&bloques_string, "]");

		return nro_bloque_str;
	}

	*ok = 1;

	t_config* config_archivo = config_create(pathParticion);

	tamanio_total = config_get_int_value(config_archivo, "TAMANIO");
	char** bloques_arr_strings = config_get_array_value(config_archivo, "BLOQUES");
	bloques_string = strdup(config_get_string_value(config_archivo, "BLOQUES"));
	int bloques_arr_strings_len = split_cant_elem(bloques_arr_strings);
	int indice_bloque_inicial = tamanio_total/atoi(BLOCK_SIZE);
	int offset_bloque_inicial = tamanio_total - (indice_bloque_inicial * atoi(BLOCK_SIZE));

	// Guardo lo del buffer en bloques
	int bytes_guardados_bloque_actual, offset_bloque_actual;
	for(nuevo_indice_bloque = indice_bloque_inicial;
			bytes > 0;
			nuevo_indice_bloque++, bytes -= bytes_guardados_bloque_actual){

		if(nuevo_indice_bloque == indice_bloque_inicial){ // Primer bloque{
			bytes_guardados_bloque_actual =  atoi(BLOCK_SIZE) - offset_bloque_inicial;
			offset_bloque_actual = offset_bloque_inicial;
		}
		else{
			bytes_guardados_bloque_actual = atoi(BLOCK_SIZE);
			offset_bloque_actual = 0;
		}

		if(nuevo_indice_bloque > bloques_arr_strings_len - 1){
			char* nro_bloque = _obtener_bloque();

			if(*ok != 1){ // No hay espacio
				log_error(g_logger, "NO HAY MAS ESPACIO");
				free(bloques_string);
				split_liberar(bloques_arr_strings);
				config_destroy(config_archivo);
				return;
			}

			_escribir_bloque(nro_bloque, offset_bloque_actual);
			free(nro_bloque);
		}
		else{
			_escribir_bloque(bloques_arr_strings[nuevo_indice_bloque], offset_bloque_actual);
		}
	}

	/* Actualizo config */
	config_set_value(config_archivo, "BLOQUES", bloques_string);
	free(bloques_string);
	char* tamanio_str = string_itoa(tamanio_total);
	config_set_value(config_archivo, "TAMANIO", tamanio_str);
	config_save(config_archivo);
	free(tamanio_str);
	split_liberar(bloques_arr_strings);
	config_destroy(config_archivo);
	return;
}

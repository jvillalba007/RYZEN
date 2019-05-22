#include "API.h"

int file_write_key_value (char* full_path, char* key, char* value){
	FILE * fPtr;
	fPtr = fopen(full_path, "a");

	if(fPtr == NULL)
	{
		printf("Unable to create file: %s \n %s \n", full_path, (char *) strerror(errno));
		return 1;
	}

	char* key_value = (char*) calloc(strlen(value) + strlen(key) + 1, sizeof(char));

	// Concatener linea a colocar
	strcat(key_value, key);
	strcat(key_value, "=");
	strcat(key_value, value);

	fputs(key_value, fPtr);
	fputs("\n", fPtr);

	fclose(fPtr);

    free(key_value);

    return 0;
}

void procesar_insert(int cant_parametros, char** parametros_no_value, char* value){

	char* table_name = parametros_no_value[1];
	char* full_path;
	full_path = generate_path(table_name, TABLES_FOLDER, ".txt");
	char* key = parametros_no_value[2];
	file_write_key_value(full_path, key, value);
	free(full_path);

}

void consola_procesar_comando(char* linea)
{

	char** parametros = string_split(linea, " ");
	int cant_parametros = split_cant_elem(parametros);

	if(string_equals_ignore_case(parametros[0],"INSERT")){

		char* value;
		value = string_extract_substring(linea, "\"", "\"");

		if (value == NULL) {
			puts("API Error: Valor no proporcionado");
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
			perror("API Error: 3 o 4 argumentos son requeridos");
		}
	}

	else if(string_equals_ignore_case(parametros[0],"SELECT")){
		if (cant_parametros == 3) {
			string_iterate_lines(parametros,puts);
		} else {
			perror("API Error: 2 argumentos son requeridos.");
		}

	}

	else if(string_equals_ignore_case(parametros[0],"CREATE")){
		if (cant_parametros == 5) {
			string_iterate_lines(parametros,puts);
		} else {
			perror("API Error: 4 argumentos son requeridos.");
		}

	}

	else if(string_equals_ignore_case(parametros[0],"DESCRIBE")){
		if (cant_parametros >= 1 && cant_parametros < 3) {
			string_iterate_lines(parametros,puts);
		} else {
			perror("API Error: ninguno o 1 argumento es requerido.");
		}

	}

	else if(string_equals_ignore_case(parametros[0],"DROP")){
		if (cant_parametros == 2) {
			string_iterate_lines(parametros,puts);
		} else {
			perror("API Error: 1 argumento es requerido.");
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


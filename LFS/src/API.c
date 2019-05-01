#include "API.h"


void procesar_insert(int cant_parametros, char** parametros_no_value, char* value){

	char* table_name = parametros_no_value[1];
	char* full_path;
	full_path = generate_path(table_name, TABLES_FOLDER, ".txt");

	printf("%s", full_path);
	free(full_path);
//	FILE * fPtr;
//	fPtr = fopen("tables/file1.txt", "w");


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

		remove_substring (linea, value); // Queda la linea sin value, solo comillas

		char** parametros_no_value = string_split(linea, " ");

		int cant_sin_value = split_cant_elem(parametros_no_value);
		// Si deja las comillas, entonces siguen siendo la misma cantida de parametros

		if (cant_sin_value >= 4 && cant_sin_value < 6) {
			split_liberar(parametros_no_value);
			procesar_insert(cant_sin_value, parametros_no_value, value);
			free(value);


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


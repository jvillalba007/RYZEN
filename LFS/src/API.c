#include "API.h"


void procesar_insert(int cant_parametros, char* linea){

	char* value;
	value = string_extract_substring(linea, "\"", "\"");

	free(value);

}

void consola_procesar_comando(char* linea)
{
	char** parametros = string_split(linea, " ");
	int cantParametros = split_cant_elem(parametros);

	if(string_equals_ignore_case(parametros[0],"INSERT")){
		if (cantParametros >= 4 && cantParametros < 6) {

			procesar_insert(cantParametros, linea);

		}else{
			perror("API Error: 3 o 4 argumentos son requeridos");
		}
	}

	else if(string_equals_ignore_case(parametros[0],"SELECT")){
		if (cantParametros == 3) {
			string_iterate_lines(parametros,puts);
		} else {
			perror("API Error: 2 argumentos son requeridos.");
		}

	}

	else if(string_equals_ignore_case(parametros[0],"CREATE")){
		if (cantParametros == 5) {
			string_iterate_lines(parametros,puts);
		} else {
			perror("API Error: 4 argumentos son requeridos.");
		}

	}

	else if(string_equals_ignore_case(parametros[0],"DESCRIBE")){
		if (cantParametros >= 1 && cantParametros < 3) {
			string_iterate_lines(parametros,puts);
		} else {
			perror("API Error: ninguno o 1 argumento es requerido.");
		}

	}

	else if(string_equals_ignore_case(parametros[0],"DROP")){
		if (cantParametros == 2) {
			string_iterate_lines(parametros,puts);
		} else {
			perror("API Error: 1 argumento es requerido.");
		}

	}

	/* Comando clear (limpiar pantalla consola) */
	else if(cantParametros == 1 && string_equals_ignore_case(parametros[0],"CLEAR")){
		system("clear");
	}

	/* Error al ingresar comando */
	else{
		printf("No se pudo reconocer el comando\n");

	}

	split_liberar(parametros);
}


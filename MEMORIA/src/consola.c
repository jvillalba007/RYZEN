/*
 * consola.c
 *
 *  Created on: 25 abr. 2019
 *      Author: utnso
 */

#include "consola.h"

void consola(){

	printf("Consola Memoria Lissandra\n");
	EXIT_PROGRAM=false;

	while(!EXIT_PROGRAM)
	{
		char* linea = console();

		if(string_equals_ignore_case(linea,"exit")) {
			EXIT_PROGRAM = true;
			shutdown(socketServidor,SHUT_RDWR);
			free(linea);
			break;
		}

		consola_procesar_comando(linea);
		free(linea);
	}
	log_info(mem_log, "FIN DE CONSOLA");
	pthread_exit(0);
}

void consola_procesar_comando(char* linea)
{
	char** parametros = string_split(linea, " ");
	int cantParametros = split_cant_elem(parametros);

	if(string_equals_ignore_case(parametros[0],"INSERT") ){

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

			linea_insert linea_ins;
			//TODO habria que adaptar el insert que vino por consola a la linea_insert para pasarleo a ejecutar_insert

			procesar_insert(cant_sin_value, parametros_no_value, value);
			free(value);
			split_liberar(parametros_no_value);


		}else{
			perror("API Error: 3 o 4 argumentos son requeridos");
	}













		/*
		char** parametros_aux = string_split(linea, "\""); //spliteo la linea con comillas
		char** parametros_estaticos  = string_split(parametros_aux[0], " "); //obtiene INSERT,[TABLA],[KEY]
		string_iterate_lines(parametros_estaticos,puts);
		puts(parametros_aux[1]); //parametros_aux[1] esta el "VALUE"
		split_liberar(parametros_estaticos);
		split_liberar(parametros_aux);
		*/
	}

	else if(cantParametros == 3 && string_equals_ignore_case(parametros[0],"SELECT")){
		string_iterate_lines(parametros,puts);
	}

	else if(cantParametros == 5 && string_equals_ignore_case(parametros[0],"CREATE")){
		string_iterate_lines(parametros,puts);
	}

	else if(cantParametros >= 1 && string_equals_ignore_case(parametros[0],"DESCRIBE")){
		string_iterate_lines(parametros,puts);
	}

	else if(cantParametros == 2 && string_equals_ignore_case(parametros[0],"DROP")){
		string_iterate_lines(parametros,puts);
	}

	else if(cantParametros == 1 && string_equals_ignore_case(parametros[0],"JOURNAL")){
		string_iterate_lines(parametros,puts);
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

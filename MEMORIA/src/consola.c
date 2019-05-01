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

		if(string_equals_ignore_case(linea,"SALIR")) {
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

	if(cantParametros >= 4 && string_equals_ignore_case(parametros[0],"INSERT")){
		char** parametros_aux = string_split(linea, "\""); //spliteo la linea con comillas
		char** parametros_estaticos  = string_split(parametros_aux[0], " "); //obtiene INSERT,[TABLA],[KEY]
		string_iterate_lines(parametros_estaticos,puts);
		puts(parametros_aux[1]); //parametros_aux[1] esta el "VALUE"
		split_liberar(parametros_estaticos);
		split_liberar(parametros_aux);
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

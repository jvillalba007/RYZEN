/*
 * consola.c
 *
 *  Created on: 25 abr. 2019
 *      Author: utnso
 */

#include "consola.h"
#include "memoria.h"

void consola(){

	printf("Consola Memoria Lissandra\n");
	EXIT_PROGRAM=false;

	while(!EXIT_PROGRAM)
	{
		char* linea = console();

		if(string_equals_ignore_case(linea,"exit")) {
			EXIT_PROGRAM = true;
			pthread_cancel(tid_server);
			pthread_kill(tid_inotify, SIGUSR1);
			pthread_kill(tid_journal, SIGUSR1);
			pthread_kill(tid_gossiping, SIGUSR1);
			free(linea);
			break;
		}
		else if ( strcmp(linea, "") != 0){
			consola_procesar_comando(linea);
		}

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

		remove_value(linea, value); // Queda la linea sin value, solo comillas

		char** parametros_no_value = string_split(linea, " ");

		int cant_sin_value = split_cant_elem(parametros_no_value);
		// Si deja las comillas, entonces siguen siendo la misma cantida de parametros

		if (cant_sin_value >= 3 && cant_sin_value < 5) {

			linea_insert linea_ins;
			linea_ins.tabla= parametros_no_value[1];
			linea_ins.key = (u_int16_t) atoi( parametros_no_value[2]);
			linea_ins.value = value;

			log_info(mem_log, "Tabla es %s",linea_ins.tabla);
			log_info(mem_log, "KEY es %d",linea_ins.key);
			log_info(mem_log, "VALUE es %s",linea_ins.value);

			if (strlen(linea_ins.value) >= maximo_value)
			{
				log_info(mem_log, "Tam Value no Permitido");
			}
			else
			{
				pthread_mutex_lock(&mutex);
				ejecutar_insert(&linea_ins);
				pthread_mutex_unlock(&mutex);
			}
			//procesar_insert(cant_sin_value, parametros_no_value, value);
			free(value);
			split_liberar(parametros_no_value);

		}else{
			perror("API Error: 3 o 4 argumentos son requeridos");
		}

	}

	else if(cantParametros == 3 && string_equals_ignore_case(parametros[0],"SELECT")){

		linea_select linea_s;
		linea_s.tabla= parametros[1];
		linea_s.key = (u_int16_t) atoi( parametros[2]);

		log_info(mem_log, "SELECT tabla: %s , key: %d",linea_s.tabla ,linea_s.key );

		pthread_mutex_lock(&mutex);
		fila_TPaginas *pagina = ejecutar_select( &linea_s );
		pthread_mutex_unlock(&mutex);

		if(pagina != NULL){
			fila_Frames frame;
			leer_de_frame( pagina->frame_registro , &frame );
			log_info(mem_log, "REQUEST DEVUELTA CON KEY: %s" , frame.value );
			puts( frame.value );
		}
		else{
			log_info(mem_log, "NO PUEDO RESOLVERSE LA REQUEST" );
		}

	}

	else if(cantParametros == 5 && string_equals_ignore_case(parametros[0],"CREATE")){
		//CREATE TABLA1 SC 4 60000
		linea_create linea_c;
		linea_c.tabla = parametros[1] ;
		linea_c.tipo_consistencia = parametros[2] ;
		linea_c.nro_particiones = (u_int8_t) atoi( parametros[3]);
		linea_c.tiempo_compactacion = (u_int32_t) atoi( parametros[4]);

		log_info(mem_log, "CREATE tabla: %s , consistencia: %s , particiones: %d , tiempo_compactacion: %d",linea_c.tabla ,linea_c.tipo_consistencia , linea_c.nro_particiones , linea_c.tiempo_compactacion );
		enviar_create_lfs( linea_c );

	}

	else if(cantParametros >= 1 && string_equals_ignore_case(parametros[0],"DESCRIBE")){
		char* tabla = cantParametros == 1 ? NULL :  parametros[1];
		log_info(mem_log, "Tira un Describe a LFS..." );
		enviar_describe_lfs(tabla);
	}

	else if(cantParametros == 2 && string_equals_ignore_case(parametros[0],"DROP")){
		char* tabla = parametros[1];
		log_info(mem_log, "DROP tabla: %s" , tabla );
		pthread_mutex_lock(&mutex);
		ejecutar_drop(tabla);
		pthread_mutex_unlock(&mutex);
	}

	else if(cantParametros == 1 && string_equals_ignore_case(parametros[0],"JOURNAL")){
		log_info(mem_log, "COMENZANDO JOURNAL..." );
		pthread_mutex_lock(&mutex);
		journal();
		pthread_mutex_unlock(&mutex);
		log_info(mem_log, "TERMINO JOURNAL..." );
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



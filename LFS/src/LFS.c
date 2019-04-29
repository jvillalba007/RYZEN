#include "LFS.h"

void crear_servidor(){

	int cliente;
	t_header *buffer = malloc( sizeof( t_header ) );

	socketServidor = socket_create_listener( NULL , lfs_config.puerto_lfs ); // @suppress("Symbol is not resolved")

	if( socketServidor < 0  ){

		log_error(g_logger, "¡Error no se pudo abrir el servidor ");
		exit(EXIT_FAILURE);
	}
	log_info(g_logger, "Se abre servidor de LFS");

	/* NUEVO CLIENTE */
	while( (cliente = socket_aceptar_conexion(socketServidor) && !EXIT_PROGRAM)  ){

		log_info(g_logger, "Se agrego una nueva conexión, socket: %d",cliente);
		/************ LEER EL HANDSHAKE ************/
		int res = recv(cliente, buffer, sizeof( t_header ) ,MSG_WAITALL);

		if (res <= 0) {
			log_error(g_logger, "¡Error en el handshake con el cliente! %d",res);
			close(cliente);
			free(buffer);
		}
		log_info(g_logger, "El emisor es: %d",buffer->emisor);

	}

	free( buffer );
	close(socketServidor);
	pthread_exit(0);

}



void consola_procesar_comando(char* linea)
{
	char** parametros = string_split(linea, " ");
	int cantParametros = split_cant_elem(parametros);

	if(string_equals_ignore_case(parametros[0],"INSERT")){
		if (cantParametros >= 4 && cantParametros < 6) {
			char** parametros_aux = string_split(linea, "\""); //spliteo la linea con comillas
			char** parametros_estaticos  = string_split(parametros_aux[0], " "); //obtiene INSERT,[TABLA],[KEY]
			string_iterate_lines(parametros_estaticos,puts);
			puts(parametros_aux[1]); //parametros_aux[1] esta el "VALUE"
			split_liberar(parametros_estaticos);
			split_liberar(parametros_aux);
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



void console_process() {


	while ( !EXIT_PROGRAM ) {

		char* buffer;

		buffer = console();

		log_info(g_logger, buffer);

		if ( 0 == strcmp(buffer, "exit") )
			{
				EXIT_PROGRAM = true;
				shutdown(socketServidor,SHUT_RDWR);
			}
		else {
			consola_procesar_comando(buffer);
		}

		free(buffer);

	}

	pthread_exit(0);

}


void liberar_memoria(){
	liberar_config(lfs_config);
	liberar_logger(g_logger);
}

int main(void) {

	EXIT_PROGRAM = false;

	iniciar_config();

	// INICIAR SOCKET SERVIDOR
	pthread_attr_t attr_server;
	pthread_attr_init(&attr_server);

	pthread_t tid_server;
	pthread_create(&tid_server, &attr_server, (void*)crear_servidor, NULL);
	//pthread_detach(tid_server);

	// INICIAR CONSOLA
	pthread_attr_t attr_consola;
	pthread_attr_init(&attr_consola);

	pthread_t tid_consola;
	pthread_create(&tid_consola, &attr_consola, (void*)console_process, NULL);

	//Esperar a que el hilo termine
	pthread_join(tid_consola, NULL);
	pthread_join(tid_server, NULL);

	liberar_memoria();


	return 0;

}

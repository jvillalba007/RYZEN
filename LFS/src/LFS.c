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

void console_process() {


	while ( !EXIT_PROGRAM ) {

		char* buffer;
		buffer = console();
		log_info(g_logger, "Comando leido: %s", buffer);

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
	iniciar_montaje();

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

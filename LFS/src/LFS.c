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
	while( (cliente = socket_aceptar_conexion(socketServidor) )  ){

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
}



void console_process(size_t bufsize) {

	bool exit_loop = true;

	while (exit_loop) {

		char* buffer;

		buffer = console();

		log_info(g_logger, buffer);

		if ( 0 == strcmp(buffer, "exit") ) exit_loop = false;
		free(buffer);

	}

	pthread_exit(0);
}

void liberar_memoria(){
	log_destroy(g_logger);
}



int main(void) {

	iniciar_config();

	// INICIAR CONSOLA
	pthread_attr_t attr_consola;
	pthread_attr_init(&attr_consola);

	pthread_t tid_consola;

	pthread_create(&tid_consola, &attr_consola, console_process, NULL);

	// INICIAR SOCKET SERVIDOR
	pthread_attr_t attr_server;
	pthread_attr_init(&attr_server);

	pthread_t tid_server;

	pthread_create(&tid_server, &attr_server, crear_servidor, NULL);



	//Esperar a que el hilo termine
	pthread_join(tid_consola, NULL);
	pthread_join(tid_server, NULL);

	liberar_memoria();

}

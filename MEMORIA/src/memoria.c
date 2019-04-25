/*
 ============================================================================
 Name        : MEMORIA.c
 Author      : RYZEN
 Version     :
 Copyright   : 2019
 Description : MODULO MEMORIA
 ============================================================================
 */

#include "memoria.h"

int main(void) {
	if (mem_initialize() == -1) {
		mem_exit();
		return EXIT_FAILURE;
	}

	imprimir_config();

	inicializar();
	/* TODO: iniciar en distintos hilos */
	/* Creo el hilo consola */
	pthread_t thread_consola;
	if(pthread_create( &thread_consola, NULL, (void*) consola, NULL) ){
		log_error(mem_log,"[MEMORIA] No pude crear el hilo para la consola");
		mem_exit();
		exit(EXIT_FAILURE);
	}
	log_info(mem_log, "[MEMORIA] Creo el hilo para la consola");
	pthread_detach(thread_consola);

	crear_servidor();
	crear_cliente_lfs();
	ejecutar_gossiping();

	liberar_mem_config(mem_config);
	mem_exit();

	return EXIT_SUCCESS;
}

/* TODO: aca hay que iniciar la lista de sedds de gossiping , segmentar y paginar la memoria */
void inicializar(){

}

void crear_servidor(){

	int cliente;
	t_header *buffer = malloc( sizeof( t_header ) );

	socketServidor = socket_create_listener( NULL , mem_config.puerto_mem );

	if( socketServidor < 0  ){

		log_error(mem_log, "¡Error no se pudo abrir el servidor ");
		exit(EXIT_FAILURE);
	}
	log_info(mem_log, "Se abre servidor de MEMORIA");

	/* NUEVO CLIENTE */
	while( (cliente = socket_aceptar_conexion(socketServidor) )  ){

		log_info(mem_log, "Se agrego una nueva conexión, socket: %d",cliente);
		/************ LEER EL HANDSHAKE ************/
		int res = recv(cliente, buffer, sizeof( t_header ) ,MSG_WAITALL);

		if (res <= 0) {
			log_error(mem_log, "¡Error en el handshake con el cliente! %d",res);
			close(cliente);
			free(buffer);
		}
		log_info(mem_log, "El emisor es: %d",buffer->emisor);

	}

	free( buffer );
}

void crear_cliente_lfs(){

	socketClienteLfs = socket_connect_to_server(mem_config.ip_LFS,  mem_config.puerto_LFS );

	if( socketClienteLfs < 0  ){

		log_error(mem_log, "¡Error no se pudo conectar con LFS");
		exit(EXIT_FAILURE);
	}

	log_info(mem_log, "Se creo el socket cliente con LFS de numero: %d", socketClienteLfs);

	t_header buffer;
	buffer.emisor=MEMORIA;
	buffer.tipo_mensaje = CONEXION ;
	buffer.payload_size = 32;

	send(socketClienteLfs, &buffer, sizeof( buffer ) , 0);
}

void ejecutar_gossiping(){

}

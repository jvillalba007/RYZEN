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
		log_destroy(mem_log);
		return EXIT_FAILURE;
	}

	imprimir_config();

	//CLIENTE CON LFS
	//crear_cliente_lfs();
	//ESTRUCTURAR/INICIALIZACION DE MEMORIA
	estructurar_memoria();

	//INICIAR SERVER
	log_info(mem_log, "[MEMORIA] Abro hilo servidor");
	pthread_t tid_server;
	pthread_create(&tid_server, NULL, (void*)crear_servidor, NULL);

	// INICIAR CONSOLA
	log_info(mem_log, "[MEMORIA] Abro hilo consola");
	pthread_t tid_consola;
	pthread_create(&tid_consola, NULL, (void*)consola, NULL);

	//GOSSIPING
	//pthread_t tid_gossiping;
	//pthread_create(&tid_gossiping, NULL, (void*)ejecutar_gossiping, NULL);



	pthread_join(tid_consola, NULL);
	pthread_join(tid_server, NULL);
	log_info(mem_log, "[MEMORIA] FINALIZO HILO CONSOLA");
	log_info(mem_log, "[MEMORIA] FINALIZO HILO SERVIDOR");

	mem_exit_global();

	return EXIT_SUCCESS;
}


void estructurar_memoria(){

	iniciar_memoria_contigua();
	iniciar_tabla_segmentos();
}

void iniciar_memoria_contigua(){
	maximo_value = 4;
	int tamanio_fila = tamanio_fila_Frames();

	log_info(mem_log, "***INICIAMOS MEMORIA CONTIGUA ****");
	cantidad_frames = mem_config.tam_mem / tamanio_fila;
	log_info(mem_log, "Tamaño Memoria: %d", mem_config.tam_mem);
	log_info(mem_log, "Tamaño de la fila: %d", tamanio_fila);
	log_info(mem_log, "Cantidad de Frames: %d", cantidad_frames);

	memoria = malloc(mem_config.tam_mem);
	memset(memoria, '\0', mem_config.tam_mem);
}

void iniciar_tabla_segmentos(){
	log_info(mem_log, "***INICIAMOS TABLA DE SEGMENTOS ****");
	tabla_segmentos = list_create();
}



int tamanio_fila_Frames(){

	return ( sizeof( int32_t ) + sizeof(u_int16_t) + maximo_value + 1 ) ;
}


void crear_servidor(){

	int cliente;
	t_header *buffer = malloc( sizeof( t_header ) );

	socketServidor = socket_create_listener( NULL , mem_config.puerto_mem );

	if( socketServidor < 0  ){

		log_error(mem_log, "¡Error no se pudo abrir el servidor!");
		free( buffer );
		close(socketServidor);
		pthread_exit(0);
	}
	log_info(mem_log, "Se abre servidor de MEMORIA");

	/* NUEVO CLIENTE */
	while( (cliente = socket_aceptar_conexion(socketServidor))  && (!EXIT_PROGRAM )  ){

			atender_request(&cliente);

	}

	log_info(mem_log, "FIN SERVIDOR");
	free( buffer );
	close(socketServidor);
	pthread_exit(0);
}

void atender_request(void* cliente_socket)
{
	int cliente = *(int *) cliente_socket;
	t_header* paquete = malloc(sizeof(t_header));

	log_info(mem_log, "Se agrego una nueva conexión, socket: %d",cliente);

	/************ LEER EL HANDSHAKE ************/
	recv(cliente, paquete, sizeof(t_header) ,MSG_WAITALL);
	log_info(mem_log, "TIPO EMISOR: %d",paquete->emisor);

	/*************************** SI EL HANDSHAKE LO HIZO UNA MEMORIA *********************************/
	if (paquete->emisor == MEMORIA) {
		log_info(mem_log, "************* NUEVA CONEXION DE MEMORIA **************");


	}

	/************************** SI EL HANDSHAKE LO HIZO KERNEL ***************************************/
	if( paquete->emisor == KERNEL ){
		log_info(mem_log, "************* NUEVA CONEXION DE KERNEL **************");

		atender_kernel(&cliente);

	}
	free(paquete);

}

void atender_kernel(int* cliente)
{
	int res ;
	t_header* paquete = malloc(sizeof(t_header));

	while ( ( res = recv(*cliente, (void*) paquete, sizeof( t_header ) ,MSG_WAITALL) )  > 0) {

		log_info(mem_log, "Se recibio request del KERNEL: %d",paquete->tipo_mensaje);

		switch (paquete->tipo_mensaje) {

		/* TODO: SELECT en shared */
		case CONEXION:{
			log_info(mem_log, "ALGORITMIA SELECT");

		}
		break;

		case DESCONEXION:{
			/* TODO: INSERT en shared */
			log_info(mem_log, "ALGORITMIA INSERT");
		}
		break;



		}

	}
	free(paquete);
	close(*cliente);
}

void crear_cliente_lfs(){
	socketClienteLfs = socket_connect_to_server(mem_config.ip_LFS,  mem_config.puerto_LFS );
	log_info(mem_log, "%d" ,socketClienteLfs);
	if( socketClienteLfs == -1  ){

		log_error(mem_log, "¡Error no se pudo conectar con LFS");
		mem_exit_simple();
		exit(EXIT_FAILURE);
	}

	log_info(mem_log, "Se creo el socket cliente con LFS de numero: %d", socketClienteLfs);

	t_header buffer;
	buffer.emisor=MEMORIA;
	buffer.tipo_mensaje = CONEXION ;
	buffer.payload_size = 32;

	send(socketClienteLfs, &buffer, sizeof( buffer ) , 0);
	/* TODO lfs nos devuelve valores, terminar de realizar */
	//maximo_value = 5;

}

void ejecutar_gossiping(){

}

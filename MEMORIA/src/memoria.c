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
	pthread_t tid_gossiping;
	pthread_create(&tid_gossiping, NULL, (void*)ejecutar_gossiping, NULL);



	pthread_join(tid_consola, NULL);
	pthread_join(tid_server, NULL);
	log_info(mem_log, "[MEMORIA] FINALIZO HILO CONSOLA");
	log_info(mem_log, "[MEMORIA] FINALIZO HILO SERVIDOR");

	liberar_tablas();
	liberar_mem_config(mem_config);
	log_info(mem_log, "[MEMORIA] LIBERO MEMORIA CONFIG");

	mem_exit();

	return EXIT_SUCCESS;
}


void estructurar_memoria(){

	iniciar_tabla_frames();
	iniciar_tabla_paginas();
	iniciar_tabla_segmentos();
}

void iniciar_tabla_frames(){
	maximo_value = 5;
	tabla_frames = list_create();
	int tamanio_fila = tamanio_fila_TFrames();

	log_info(mem_log, "***INICIAMOS TABLA DE FRAMES ****");
	cantidad_frames = mem_config.tam_mem / tamanio_fila;
	log_info(mem_log, "Tamaño Memoria: %d", mem_config.tam_mem);
	log_info(mem_log, "Tamaño de la fila: %d", tamanio_fila);
	log_info(mem_log, "Cantidad de Frames: %d", cantidad_frames);

	int i;

	void _agregar_nueva_fila(){
		void* nueva_fila = calloc(tamanio_fila,sizeof(char));

		int posicion = 0;
		fila_TFrames* fila_frame = malloc(sizeof(fila_TFrames));
		fila_frame->timestamp = nueva_fila;
		posicion+=sizeof( int32_t );
		fila_frame->key = nueva_fila + posicion;
		posicion+=sizeof(u_int16_t);
		fila_frame->value = nueva_fila + posicion;

		list_add(tabla_frames, (void*) fila_frame);
	}

	for(i = 0; i < cantidad_frames; i++){
		_agregar_nueva_fila();
	}
}

void iniciar_tabla_paginas(){

	log_info(mem_log, "***INICIAMOS TABLA DE PAGINAS ****");
	log_info(mem_log, "Cantidad de Paginas: %d", cantidad_frames);
	tabla_paginas = list_create();

	void _agregar_nueva_fila(int i){
		fila_TPaginas* fila_pagina = malloc(sizeof(fila_TPaginas));
		fila_pagina->numero_pagina = i;
		fila_pagina->modificado = 0;

		list_add(tabla_paginas, (void*) fila_pagina);
	}

	int i;
	for(i = 0; i < cantidad_frames; i++){
		_agregar_nueva_fila(i);
	}
}

void iniciar_tabla_segmentos(){
	tabla_segmentos = list_create();
}



int tamanio_fila_TFrames(){

	return ( sizeof( int32_t ) + sizeof(u_int16_t) + maximo_value ) ;
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
	while( (cliente = socket_aceptar_conexion(socketServidor)  && !EXIT_PROGRAM )  ){

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

	log_info(mem_log, "FIN SERVIDOR");
	free( buffer );
	close(socketServidor);
	pthread_exit(0);
}

void crear_cliente_lfs(){
	socketClienteLfs = socket_connect_to_server(mem_config.ip_LFS,  mem_config.puerto_LFS );
	log_info(mem_log, "%d" ,socketClienteLfs);
	if( socketClienteLfs == -1  ){

		log_error(mem_log, "¡Error no se pudo conectar con LFS");
		liberar_mem_config(mem_config);
		log_info(mem_log, "[MEMORIA] LIBERO MEMORIA CONFIG");
		mem_exit();
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

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

	frames_ocupados=2;

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

			linea_insert linea;
			linea.tabla =strdup( "TEST" );
			linea.key= 1 ;
			linea.value= strdup( "Hola" ) ;

			fila_TSegmentos *segmento = obtener_segmento( linea.tabla );

			if( segmento == NULL ) segmento = crear_segmento( linea.tabla );


			fila_TPaginas *pagina=NULL;
			if( !list_is_empty(segmento->paginas )) pagina = obtener_pagina_segmento( segmento , linea.key );


			if( pagina != NULL ) {

				actualizar_pagina( pagina , linea );
				log_info(mem_log, "SE ACTUALIZO LA PAGINA CON KEY: %s" , linea.value  ) ;
			}
			else{

				char* frame = obtener_frame_libre();
				log_info(mem_log, "Numero de frame obtenido: %d" , (int)(frame-memoria)   / tamanio_fila_Frames()   ) ;

				fila_Frames linea_frame = inicializar_fila_frame(linea ) ;
				log_info(mem_log, "Se iniciliza frame con key: %d" , linea_frame.key  ) ;

				escribir_en_frame( frame , linea_frame );
				fila_TPaginas* pagina = crear_pagina( segmento , frame );
				log_info(mem_log, "SE CREA PAGINA EN EL SEGMENTO. El bit modificado es: %d" , pagina->modificado  ) ;

				fila_Frames registro;
				leer_de_frame( pagina->frame_registro , &registro );
				log_info(mem_log, "LA INFORMACION DEL FRAME INSERTADO ES key: %d , value: %s  , timestamp: %d" , registro.key , registro.value , registro.timestamp ) ;
				log_info(mem_log, "LA CANTIDAD DE PAGINAS DEL SEGMENTO ES: %d" , list_size(segmento->paginas )  ) ;
			}



			free(linea.tabla);
			free(linea.value);
		}
		break;

		}

	}
	free(paquete);
	close(*cliente);
}


void actualizar_pagina( fila_TPaginas* pagina , linea_insert linea ){

	pagina->ultimo_uso=0; //TODO: verificar que poner aca
	pagina->modificado=1;
	fila_Frames linea_frame = inicializar_fila_frame( linea ) ;
	escribir_en_frame(  pagina->frame_registro , linea_frame );
}


fila_TPaginas* crear_pagina(  fila_TSegmentos* segmento , char* frame ){

	fila_TPaginas *pagina = malloc( sizeof( fila_TPaginas ) );

	pagina->frame_registro = frame;
	pagina->modificado = 1;
	pagina->numero_pagina =list_size( segmento->paginas);
	pagina->ultimo_uso=0;//TODO: verificar que poner aca
	list_add( segmento->paginas , pagina );

	return pagina;
}



char *obtener_frame_libre(){

	char* frame;

	if( frames_ocupados < cantidad_frames ){

		frame= memoria+ ( tamanio_fila_Frames() * frames_ocupados );
		frames_ocupados++;
	}
	else
	{
		frame = ejecutar_lru();
	}

	return frame;
}


fila_TSegmentos*  crear_segmento( char *nombre_tabla ){

	fila_TSegmentos *segmento = malloc( sizeof( fila_TSegmentos ) );

	segmento->nombre_tabla = strdup( nombre_tabla );
	segmento->paginas = list_create();
	list_add( tabla_segmentos , segmento );

	return segmento;
}


fila_TSegmentos* obtener_segmento( char *nombre_tabla ){

	bool buscar_segmento(fila_TSegmentos *s) {

		if(  string_equals_ignore_case(  s->nombre_tabla , nombre_tabla ) ) return true;
		return false;
	}

	fila_TSegmentos *segmento = NULL;
	segmento = list_find( tabla_segmentos , (void*)buscar_segmento );

	return segmento;
}

fila_TPaginas *obtener_pagina_segmento( fila_TSegmentos *segmento , u_int16_t key ){

	bool buscar_pagina(fila_TPaginas *p) {

		fila_Frames fila_f;
		leer_de_frame(p->frame_registro , &fila_f );

		if( fila_f.key == key ) return true;
		return false;
	}

	fila_TPaginas *pagina= NULL;
	pagina = list_find( segmento->paginas , (void*)buscar_pagina );

	return pagina;
}


fila_Frames inicializar_fila_frame( linea_insert linea ){

	fila_Frames fila_frame;

	fila_frame.key= linea.key;
	fila_frame.timestamp= 450; //TODO: funcion de timestamp
	fila_frame.value= linea.value;

	return fila_frame;
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

char* ejecutar_lru(){

	return NULL;
}

void ejecutar_gossiping(){

}

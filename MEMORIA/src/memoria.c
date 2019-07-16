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

void handler(int id) {

}

void assignHandler() {
	struct sigaction sa = {0};
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
}

void retardoLFS(){

	struct timespec ts;
	ts.tv_sec = mem_config.retardo_fs / 1000;
	ts.tv_nsec = (mem_config.retardo_fs  % 1000) * 1000000;

	nanosleep(&ts, NULL);
}

int main(int argc, char *argv[]) {

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mutex_socket, NULL);

	if (mem_initialize(argv[1]) == -1) {
		log_destroy(mem_log);
		return EXIT_FAILURE;
	}

	imprimir_config();
	log_info(mem_log, "[MEMORIA] ARCHIVO RECIBIDO: %s", fileCFG);

	//CLIENTE CON LFS
	crear_cliente_lfs();

	//ESTRUCTURAR/INICIALIZACION DE MEMORIA
	estructurar_memoria();

	// INICIAR HILO INOTIFY
	log_info(mem_log, "[MEMORIA] Abro hilo INOTIFY");
	pthread_create(&tid_inotify, NULL, (void*)inotify_config, NULL);

	// INICIAR CONSOLA
	log_info(mem_log, "[MEMORIA] Abro hilo consola");
	pthread_t tid_consola;
	pthread_create(&tid_consola, NULL, (void*)consola, NULL);

	//INICIAR SERVER
	log_info(mem_log, "[MEMORIA] Abro hilo servidor");
	pthread_create(&tid_server, NULL, (void*)crear_servidor, NULL);

	//GOSSIPING
	log_info(mem_log, "[MEMORIA] Abro hilo GOSSIPING");
	pthread_create(&tid_gossiping, NULL, (void*)hilo_gossiping, NULL);

	// INICIAR JOURNAL
	log_info(mem_log, "[MEMORIA] Abro hilo JOURNAL");
	pthread_create(&tid_journal, NULL, (void*)hilo_journal, NULL);




	pthread_join(tid_consola, NULL);
	pthread_join(tid_journal, NULL);
	pthread_join(tid_gossiping, NULL);
	pthread_join(tid_server, NULL);
	pthread_join(tid_inotify, NULL);
	log_info(mem_log, "[MEMORIA] FINALIZO HILO INOTIFY");
	log_info(mem_log, "[MEMORIA] FINALIZO HILO CONSOLA");
	log_info(mem_log, "[MEMORIA] FINALIZO HILO JOURNAL");
	log_info(mem_log, "[MEMORIA] FINALIZO HILO GOSSIPING");
	log_info(mem_log, "[MEMORIA] FINALIZO HILO SERVIDOR");

	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&mutex_socket);
	mem_exit_global();

	return EXIT_SUCCESS;
}


void estructurar_memoria(){

	frames_ocupados=0;

	iniciar_tabla_memorias();
	iniciar_memoria_contigua();
	iniciar_tabla_segmentos();
}

void iniciar_memoria_contigua(){
	int tamanio_fila = tamanio_fila_Frames();

	log_info(mem_log, "***INICIAMOS MEMORIA CONTIGUA ****");
	cantidad_frames = mem_config.tam_mem / tamanio_fila;
	log_info(mem_log, "Tamaño Memoria: %d", mem_config.tam_mem);
	log_info(mem_log, "Tamaño del value: %d", maximo_value);
	log_info(mem_log, "Tamaño de la fila: %d", tamanio_fila);
	log_info(mem_log, "Cantidad de Frames: %d", cantidad_frames);

	bitMapStr = calloc(ceiling(cantidad_frames, 8), 1);
	bitmap_frames = bitarray_create_with_mode(bitMapStr, ceiling(cantidad_frames, 8), MSB_FIRST);

	memoria = malloc(mem_config.tam_mem);
	memset(memoria, '\0', mem_config.tam_mem);
}

void iniciar_tabla_segmentos(){
	log_info(mem_log, "***INICIAMOS TABLA DE SEGMENTOS ****");
	tabla_segmentos = list_create();
}


void iniciar_tabla_memorias(){

	tabla_memorias = list_create();

	//Se agrega a si misma
	t_memoria *memoria_seed = malloc( sizeof( t_memoria ) );
	memoria_seed->ip = strdup(mem_config.ip_mem );
	memoria_seed->puerto= strdup( mem_config.puerto_mem );
	memoria_seed->activa=1;
	memoria_seed->socket=-1;
	memoria_seed->numero_memoria=mem_config.memory_number;

	log_info(mem_log, "Se agrega memoria numero:%d en tabla de memorias con puerto: %s e ip: %s" ,memoria_seed->numero_memoria , memoria_seed->puerto , memoria_seed->ip);
	list_add( tabla_memorias , memoria_seed );
	memoria_seed = NULL;

	//Agrego seeds por archivo de config
	int i = 0;
	for(; mem_config.ip_SEEDS[i] != NULL;i++){

		memoria_seed = malloc( sizeof( t_memoria ) );
		memoria_seed->ip= strdup(mem_config.ip_SEEDS[i]);
		memoria_seed->puerto= strdup(mem_config.puerto_SEEDS[i]);
		memoria_seed->activa=0;
		memoria_seed->socket=-1;
		memoria_seed->numero_memoria=-1;

		log_info(mem_log, "Se agrega memoria en tabla de memorias con puerto: %s e ip: %s" , memoria_seed->puerto , memoria_seed->ip);
		list_add( tabla_memorias , memoria_seed );

		memoria_seed = NULL;
	}

}


int tamanio_fila_Frames(){

	return ( sizeof( uint64_t ) + sizeof(u_int16_t) + maximo_value ) ;
}


void crear_servidor(){

	if((socketServidor = socket_create_listener(mem_config.ip_mem,mem_config.puerto_mem)) == -1){
		log_error(mem_log, "¡Error no se pudo abrir el servidor!");
		pthread_exit(0);
	}

	log_info(mem_log, "Se abre servidor de MEMORIA");
	log_info(mem_log, "[MEMORIA] Escucho en el socket %d. Mi IP es: %s",socketServidor, mem_config.ip_mem);
	socket_start_listening_select(socketServidor, atender_request, 0);
	log_info(mem_log, "FIN SERVIDOR");

	close(socketServidor);
	pthread_exit(0);
}

int atender_request(int cliente, t_msg* msg)
{

	if(msg->header->emisor == DESCONOCIDO){
			log_info(mem_log, "[MEMORIA] Se Agrego Nueva Conexion");
			return 1;
	}

	/*************************** SI EL HANDSHAKE LO HIZO UNA MEMORIA *********************************/
	if (msg->header->emisor == MEMORIA) {
		log_info(mem_log, "************* NUEVA CONEXION DE MEMORIA **************");
		log_info(mem_log, "[Memoria] EVENTO: Emisor: %d, Tipo: %d, Tamanio: %d",msg->header->emisor,msg->header->tipo_mensaje,msg->header->payload_size);

		if(msg->header->tipo_mensaje == GOSSIPING)
		{
			log_info(mem_log, "Se recibe tabla de gossiping de una memoria");
			char* data;
			data = malloc(msg->header->payload_size);
			memcpy((void*) data, msg->payload, msg->header->payload_size);//TENER EN CUENTA SI HAY ERRORES...
			t_list* mems = deserializar_memorias(data);

			agregar_memorias_gossiping( mems );
			free(data);
			liberar_tabla_memorias(mems);


			//ENVIO INFO DE TABLA
			t_header buffer;
			buffer.emisor=MEMORIA;
			buffer.tipo_mensaje =  GOSSIPING;

			t_list* memorias_activas = get_memorias_activas( tabla_memorias );
			char* data_send = serializar_memorias(memorias_activas,&buffer.payload_size);

			send(cliente, &buffer, sizeof( buffer ) , 0);
			send(cliente, data_send, buffer.payload_size , 0);
			free(data_send);
		}

		if(msg->header->tipo_mensaje == CONEXION)
		{
			log_info(mem_log, "Se recibe mensaje de conexion de una memoria");
			send(cliente, &mem_config.memory_number, sizeof( int ) , 0);
		}

		if(msg->header->tipo_mensaje == DESCONEXION)
		{
			log_error(mem_log, "[Memoria] Se desconecto una Memoria");
			return -1;
		}

	}

	/************************** SI EL HANDSHAKE LO HIZO KERNEL ***************************************/
	if( msg->header->emisor == KERNEL ){
		log_info(mem_log, "************* NUEVA CONEXION DE KERNEL **************");
		log_info(mem_log, "[Memoria] EVENTO: Emisor: %d, Tipo: %d, Tamanio: %d",msg->header->emisor,msg->header->tipo_mensaje,msg->header->payload_size);
		return atender_kernel(cliente,msg);

	}

	return 1;
}

int atender_kernel(int cliente, t_msg* msg)
{
		char* data;
		switch (msg->header->tipo_mensaje) {

		case SELECT:{
			log_info(mem_log, "ALGORITMIA SELECT");
			linea_select linea;
			data = malloc(msg->header->payload_size);
			memcpy((void*) data, msg->payload, msg->header->payload_size);//TENER EN CUENTA SI HAY ERRORES...
			deserializar_select(data,&linea);
			free(data);

			pthread_mutex_lock(&mutex);
			fila_TPaginas* pagina = ejecutar_select(&linea);
			pthread_mutex_unlock(&mutex);

			if(pagina == NULL)
			{
				t_header paquete;
				paquete.emisor=MEMORIA;
				paquete.tipo_mensaje = EJECUCIONERROR;
				send(cliente, &paquete,sizeof(t_header),0);
				break;
			}

			free(linea.tabla);

			fila_Frames frame;
			leer_de_frame( pagina->frame_registro , &frame );

			t_header paquete;
			paquete.emisor=MEMORIA;
			paquete.tipo_mensaje = EJECUCIONOK;

			linea_response_select linears;
			linears.value = strdup(frame.value);
			linears.timestamp = frame.key;

			char* data = serializar_response_select(linears,&paquete.payload_size);

			send(cliente, &paquete,sizeof(t_header),0);
			send(cliente, data, paquete.payload_size, 0);
			free(data);
			free(linears.value);

		}
		break;

		case INSERT:{
			log_info(mem_log, "ALGORITMIA INSERT");
			linea_insert linea;
			data = malloc(msg->header->payload_size);
			memcpy((void*) data, msg->payload, msg->header->payload_size);//TENER EN CUENTA SI HAY ERRORES..
			deserializar_insert(data,&linea);
			free(data);

			t_header paquete;
			if(strlen(linea.value) >= maximo_value)
			{
				paquete.tipo_mensaje = EJECUCIONERROR; //FALLO
				log_info(mem_log, "Tam Value no Permitido");
			}
			else
			{
				paquete.tipo_mensaje = EJECUCIONOK; //OKEY
				pthread_mutex_lock(&mutex);
				ejecutar_insert(&linea);
				pthread_mutex_unlock(&mutex);
			}

			free(linea.tabla);
			free(linea.value);

			paquete.emisor = MEMORIA;
			send(cliente, &paquete,sizeof(t_header),0);
		}
		break;

		case CREATE:{
			log_info(mem_log, "ALGORITMIA CREATE");

			int retorno;
			verificarSocketLFS();

			retorno = send(socketClienteLfs,msg->header,sizeof(t_header),0);
			t_header paquete;

			if(retorno == -1)
			{
				log_error(mem_log, "LFS DEAD..."  );
				pthread_mutex_lock(&mutex_socket);
				socketClienteLfs = -1;
				pthread_mutex_unlock(&mutex_socket);

				paquete.tipo_mensaje = EJECUCIONERROR;
				paquete.emisor = MEMORIA;
				send(cliente, &paquete,sizeof(t_header),0);
				break;
			}
			send(socketClienteLfs,msg->payload,msg->header->payload_size,0);


			retorno = recv(socketClienteLfs, &paquete, sizeof(t_header), MSG_WAITALL);

			if(retorno == -1)
			{
				log_error(mem_log, "LFS DEAD..."  );
				pthread_mutex_lock(&mutex_socket);
				socketClienteLfs = -1;
				pthread_mutex_unlock(&mutex_socket);
				paquete.tipo_mensaje = EJECUCIONERROR;
			}

			retardoLFS();

			paquete.emisor = MEMORIA;
			send(cliente, &paquete,sizeof(t_header),0);
		}
		break;

		case DROP:{
			log_info(mem_log, "ALGORITMIA DROP");
			char* tabla;
			data = malloc(msg->header->payload_size);
			memcpy((void*) data, msg->payload, msg->header->payload_size);//TENER EN CUENTA SI HAY ERRORES..
			tabla = deserializar_string(data);
			pthread_mutex_lock(&mutex);
			int resultadoDROP = ejecutar_drop(tabla);
			pthread_mutex_unlock(&mutex);
			free(data);
			free(tabla);

			t_header paquete;
			paquete.emisor = MEMORIA;
			paquete.tipo_mensaje = resultadoDROP;
			send(cliente, &paquete,sizeof(t_header),0);
		}
		break;

		case DESCRIBE:{
			log_info(mem_log, "ALGORITMIA DESCRIBE");
			t_header paquete;
			int retorno;
			verificarSocketLFS();

			retorno = send(socketClienteLfs,msg->header,sizeof(t_header),0);

			if(retorno == -1)
			{
				log_error(mem_log, "LFS DEAD..."  );
				pthread_mutex_lock(&mutex_socket);
				socketClienteLfs = -1;
				pthread_mutex_unlock(&mutex_socket);
				paquete.tipo_mensaje = EJECUCIONERROR;
				send(cliente, &paquete,sizeof(t_header),0);
				break;
			}

			send(socketClienteLfs,msg->payload,msg->header->payload_size,0);

			retorno = recv(socketClienteLfs, &paquete, sizeof(t_header), MSG_WAITALL);
			paquete.emisor = MEMORIA;

			if(paquete.tipo_mensaje == EJECUCIONERROR || retorno == -1)
			{
				log_error(mem_log, "LFS DEAD..."  );
				pthread_mutex_lock(&mutex_socket);
				socketClienteLfs = -1;
				pthread_mutex_unlock(&mutex_socket);
				paquete.tipo_mensaje = EJECUCIONERROR;
				send(cliente, &paquete,sizeof(t_header),0);
				break;
			}

			char *data = malloc(paquete.payload_size);
			recv(socketClienteLfs, data, paquete.payload_size, MSG_WAITALL);

			retardoLFS();

			send(cliente, &paquete,sizeof(t_header),0);
			send(cliente, data, paquete.payload_size, 0);
			free(data);
		}
		break;

		case GOSSIPING:{
			log_info(mem_log, "GOSSIPING");

			t_header paquete;
			paquete.emisor=MEMORIA;
			paquete.tipo_mensaje = GOSSIPING;

			/*ENVIO MEMORIAS ACTIVAS*/
			t_list* memorias_activas = get_memorias_activas( tabla_memorias );

			char* data = serializar_memorias(memorias_activas,&paquete.payload_size);
			send(cliente, &paquete, sizeof(t_header) , 0);
			send(cliente, data, paquete.payload_size , 0);
			free(data);
		}
		break;

		case JOURNAL:{
			log_info(mem_log, "COMENZANDO JOURNAL..." );
			pthread_mutex_lock(&mutex);
			journal();
			pthread_mutex_unlock(&mutex);
			log_info(mem_log, "TERMINO JOURNAL..." );

		}
		break;

		case CONEXION:{
			log_info(mem_log, "Se Conecta KERNEL");
			send(cliente, &mem_config.memory_number,sizeof(int),0);
		}
		break;

		case DESCONEXION:{
			log_error(mem_log, "[Memoria] Se desconecto KERNEL");
			return -1;
		}
		break;

		}
	return 1;
}

int ejecutar_drop( char* tabla ){

	log_info(mem_log, "***************INICIA DROP**********************" ) ;

	bool buscar_segmento(fila_TSegmentos *s) {

		if(  string_equals_ignore_case(  s->nombre_tabla , tabla ) ) return true;
		return false;
	}

	fila_TSegmentos* segmento = obtener_segmento( tabla );

	if( segmento == NULL )
	{
		log_info(mem_log, "SEGMENTO DE TABLA: %s NO EXISTE" , tabla );
	}
	else
	{
		drop_tabla_paginas(segmento,tamanio_fila_Frames());
		list_remove_by_condition(tabla_segmentos,(void*)buscar_segmento);
		free( segmento->nombre_tabla );
		free(segmento);
		log_info(mem_log, "LIBERADO SEGMENTO");
	}

	return enviar_drop_lfs( tabla );

}

fila_TPaginas* ejecutar_select( linea_select* linea ){

	log_info(mem_log, "***************INICIA SELECT**********************" ) ;

	fila_TSegmentos *segmento = obtener_segmento( linea->tabla );

	if( segmento == NULL ) segmento = crear_segmento( linea->tabla );

	log_info(mem_log, "SEGMENTO DE TABLA: %s" , segmento->nombre_tabla  ) ;

	fila_TPaginas *pagina=NULL;
	if( !list_is_empty(segmento->paginas )) pagina = obtener_pagina_segmento( segmento , linea->key );

	if( pagina != NULL )
	{
		log_info(mem_log, "PAGINA ENCONTRADA ACTUALIZAMOS ULTIMO USO" ) ;
		//si encuentra pagina actualizo ultimo uso
		pagina->ultimo_uso = getCurrentTime();
		log_info(mem_log, "SE ACTUALIZO ULTIMO USO DE LA PAGINA CON KEY: %d" , linea->key  ) ;
	}
	else
	{
		log_info(mem_log, "PAGINA NO ENCONTRADA HACAEMOS REQUEST A LFS Y OBTENEMOS FRAME DISPONIBLE" ) ;

		//TODO: hacer request a lfs y recibir la info
		char* frame = obtener_frame_libre();

		if(frame != NULL){

			log_info(mem_log, "****HAY FRAMES DISPONIBLES***") ;
			log_info(mem_log, "Numero de frame obtenido: %d" , (int)(frame-memoria)   / tamanio_fila_Frames()   ) ;


			linea_response_select* linea_response = enviar_select_lfs( linea );

			if( linea_response == NULL )
			{
				int nro_frame = (int) (frame-memoria) / tamanio_fila_Frames();
				bitarray_clean_bit(bitmap_frames, nro_frame);
				frames_ocupados--;
				log_info(mem_log, "****Fallo el Select libero Frame %d Reservado***", nro_frame);
				return NULL;
			}

			//combierto el linea entrante a linea_insert para incializarlo
			linea_insert linea_ins;
			linea_ins.tabla= linea->tabla;
			linea_ins.key = linea->key;
			linea_ins.value = linea_response->value;
			fila_Frames linea_frame = inicializar_fila_frame( linea_ins ) ;
			//TODO; decidir si agregar parametro a esta funcion o usar una nueva funcion o actualizar el timestamp luego de inicializar el frame
			linea_frame.timestamp=linea_response->timestamp;

			log_info(mem_log, "Se iniciliza frame con key: %d" , linea_frame.key  ) ;

			escribir_en_frame( frame , linea_frame );

			//TODO: no deberia crear pagina si escrbir frame -1
			pagina = crear_pagina( segmento , frame , 0 );
			log_info(mem_log, "SE CREA PAGINA EN EL SEGMENTO. El bit modificado es: %d" , pagina->modificado  ) ;
			log_info(mem_log, "PAGINA N°: %d" , pagina->numero_pagina) ;
			log_info(mem_log, "ULTIMO USO: %" PRIu64, pagina->ultimo_uso  ) ;

			free(linea_response->value);
			free(linea_response);

		}
		else{

			log_info(mem_log, "****NO HAY FRAMES DISPONIBLES SE RECHAZA REQUEST***");
			log_info(mem_log, "---FALLO ALGORITMO DE REEMPLAZO. HAY QUE HACER JOURNAL---");
			log_info(mem_log, "COMENZANDO JOURNAL..." );
			journal();
			log_info(mem_log, "TERMINO JOURNAL..." );

			pagina = ejecutar_select(linea );
		}

	}

	return pagina;
}


void ejecutar_insert(linea_insert* linea){

	log_info(mem_log, "***************INICIA INSERT**********************" ) ;

	fila_TSegmentos *segmento = obtener_segmento( linea->tabla );

	if( segmento == NULL ) segmento = crear_segmento( linea->tabla );

	log_info(mem_log, "SEGMENTO DE TABLA: %s" , segmento->nombre_tabla  ) ;

	fila_TPaginas *pagina=NULL;
	if( !list_is_empty(segmento->paginas )) pagina = obtener_pagina_segmento( segmento , linea->key );


	if( pagina != NULL ) {

		actualizar_pagina( pagina , *linea );
		log_info(mem_log, "SE ACTUALIZO LA PAGINA CON KEY: %s" , linea->value  ) ;
	}
	else{

	char* frame = obtener_frame_libre();

	if(frame != NULL){
		log_info(mem_log, "Numero de frame obtenido: %d" , (int)(frame-memoria)   / tamanio_fila_Frames()   ) ;

		fila_Frames linea_frame = inicializar_fila_frame(*linea ) ;
		log_info(mem_log, "Se iniciliza frame con key: %d" , linea_frame.key  ) ;

		escribir_en_frame( frame , linea_frame );
		fila_TPaginas* pagina = crear_pagina( segmento , frame , 1 );
		log_info(mem_log, "SE CREA PAGINA EN EL SEGMENTO. El bit modificado es: %d" , pagina->modificado  ) ;
		log_info(mem_log, "PAGINA N°: %d" , pagina->numero_pagina) ;
		log_info(mem_log, "ULTIMO USO: %" PRIu64, pagina->ultimo_uso  ) ;

		fila_Frames registro;
		leer_de_frame( pagina->frame_registro , &registro );
		log_info(mem_log, "LA INFORMACION DEL FRAME INSERTADO ES key: %d , value: %s  , timestamp: %" PRIu64 , registro.key , registro.value , registro.timestamp ) ;
		log_info(mem_log, "LA CANTIDAD DE PAGINAS DEL SEGMENTO ES: %d" , list_size(segmento->paginas )  ) ;
		log_info(mem_log, "***************FIN INSERT**********************" ) ;
	}
	else{
		log_info(mem_log, "---FALLO ALGORITMO DE REEMPLAZO. HAY QUE HACER JOURNAL---");
		log_info(mem_log, "COMENZANDO JOURNAL..." );
		journal();
		log_info(mem_log, "TERMINO JOURNAL..." );

		ejecutar_insert(linea);
		}
	}
}

void actualizar_pagina( fila_TPaginas* pagina , linea_insert linea ){
	pagina->ultimo_uso = getCurrentTime();
	pagina->modificado=1;
	fila_Frames linea_frame = inicializar_fila_frame( linea ) ;
	escribir_en_frame(  pagina->frame_registro , linea_frame );
}


fila_TPaginas* crear_pagina(  fila_TSegmentos* segmento , char* frame , int8_t modificado ){

	fila_TPaginas *pagina = malloc( sizeof( fila_TPaginas ) );

	pagina->frame_registro = frame;
	pagina->modificado = modificado;
	pagina->numero_pagina =list_size( segmento->paginas);
	pagina->ultimo_uso = getCurrentTime();
	list_add( segmento->paginas , pagina );

	return pagina;
}

int SPA_obtener_frame(){
	int i = 0;
	int frame_encontrado = 0;

	while(i < cantidad_frames  && !frame_encontrado){
		if(bitarray_test_bit(bitmap_frames, i) == 0){ // Frame disponible
			bitarray_set_bit(bitmap_frames, i); // Lo selecciono como no disponible
			frame_encontrado = 1;
		}
		else
			i++;
	}

	return i;
}

char *obtener_frame_libre(){

	char* frame;

	if( frames_ocupados < cantidad_frames ){
		frame = memoria + (tamanio_fila_Frames() * SPA_obtener_frame());
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

	if(!list_is_empty(tabla_segmentos))
	{
	segmento = list_find( tabla_segmentos , (void*)buscar_segmento );
	}

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
	if(!list_is_empty(segmento->paginas))
	{
	pagina = list_find( segmento->paginas , (void*)buscar_pagina );
	}
	return pagina;
}


fila_Frames inicializar_fila_frame( linea_insert linea ){

	fila_Frames fila_frame;

	fila_frame.key= linea.key;
	fila_frame.timestamp=getCurrentTime();
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
	buffer.payload_size = 0;

	send(socketClienteLfs, &buffer, sizeof( buffer ) , 0);
	/* TODO lfs nos devuelve valores, terminar de realizar */
	recv(socketClienteLfs, &maximo_value, sizeof(int), MSG_WAITALL);

	log_info(mem_log, "MAXIMO VALUE: %d", maximo_value);

}

char* ejecutar_lru(){

	fila_TSegmentos* segmento = NULL;
	fila_TPaginas* pagina = NULL;
	char* frame = NULL;

	uint64_t minimun;
	int posicion;
	int pos;

	minimun = getCurrentTime();

	void algoritmo_reemplazo(fila_TSegmentos* un_segmento)
	{
		void findLRU(fila_TPaginas* pagina_segmento,int* pos){

				/*Si encontro una pagina que no esta modificada, y es vieja (ultimo_uso menor global)
				* entonces lo eligo como victima para el LRU. */
				if((pagina_segmento->ultimo_uso < minimun) && (pagina_segmento->modificado != 1)){
					minimun = pagina_segmento->ultimo_uso;

					segmento = un_segmento;
					pagina = pagina_segmento;
					posicion = *(pos);
				}
		}

		//Busco con LRU a toda la tabla de paginas, si tiene paginas...
		if(!list_is_empty(un_segmento->paginas))
		{
			list_iterate_pos(un_segmento->paginas,(void*)findLRU,&pos);
		}
	}

	//Aplico el algoritmo de reemplazo a todos los segmentos(tablas)
	list_iterate(tabla_segmentos,(void*)algoritmo_reemplazo );

	//Si tuve exito con LRU segmento y pagina son distintos de NULL
	if((segmento != NULL) && (pagina != NULL))
	{
		log_info(mem_log,"---ENCONTRE FRAME POR LRU---");
		frame = pagina->frame_registro;
		log_info(mem_log, "FRAME POR LRU ES: %d", ((frame-memoria)   / tamanio_fila_Frames()   ));

		log_info(mem_log,"SE QUITO AL SEGMENTO: %s",segmento->nombre_tabla);
		log_info(mem_log,"LA PAGINA N°: %d",pagina->numero_pagina);
		list_remove(segmento->paginas,posicion);
		free(pagina);
	}

	//Si Frame es NULL quiere decir que hay que hacer JOURNAL (fallo LRU)
	return frame;
}


linea_response_select* enviar_select_lfs( linea_select *linea ){

	//TODO: hacer la request de select al lfs
	log_info(mem_log, "REQUEST DE SELECT A LFS"  ) ;

	t_header paquete;

	char* buffer = serializar_select(*linea, &paquete.payload_size);
	paquete.emisor = MEMORIA;
	paquete.tipo_mensaje = SELECT;

	int retorno;
	verificarSocketLFS();

	retorno = send(socketClienteLfs, &paquete, sizeof(t_header), 0);

	if(retorno == -1)
	{
		free(buffer);
		log_error(mem_log, "LFS DEAD..."  );
		pthread_mutex_lock(&mutex_socket);
		socketClienteLfs = -1;
		pthread_mutex_unlock(&mutex_socket);
		return NULL;
	}

	send(socketClienteLfs, buffer, paquete.payload_size, 0);
	free(buffer);

	t_header paqueteLFS;
	retorno = recv(socketClienteLfs, &paqueteLFS, sizeof(t_header), MSG_WAITALL);

	if(paqueteLFS.tipo_mensaje == EJECUCIONERROR || retorno == -1) //FALLO
	{
		log_error(mem_log, "LFS DEAD..."  );
		pthread_mutex_lock(&mutex_socket);
		socketClienteLfs = -1;
		pthread_mutex_unlock(&mutex_socket);
		return NULL;
	}

	char *data = malloc(paqueteLFS.payload_size);
	recv(socketClienteLfs, data, paqueteLFS.payload_size, MSG_WAITALL);

	retardoLFS();

	linea_response_select* linea_response = malloc(sizeof(linea_response_select));
	deserializar_response_select(data, linea_response);
	free(data);

    return linea_response;
}

void enviar_describe_lfs( char *tabla ){

	t_header paquete;
	paquete.emisor = MEMORIA;
	paquete.tipo_mensaje = DESCRIBE;
	paquete.payload_size = 0;

	int retorno;
	verificarSocketLFS();

	if(tabla != NULL)
	{
		char* buffer;
		buffer = serializar_string(tabla, &(paquete.payload_size));
		retorno = send(socketClienteLfs, &paquete, sizeof(t_header), 0);
		(retorno == -1) ? 0 : send(socketClienteLfs, buffer, paquete.payload_size, 0);
		free(buffer);
	}
	else
	{
		retorno = send(socketClienteLfs, &paquete, sizeof(t_header), 0);
	}

	if(retorno == -1)
	{
		pthread_mutex_lock(&mutex_socket);
		socketClienteLfs = -1;
		pthread_mutex_unlock(&mutex_socket);
		return;
	}

	t_header paqueteLFS;
	retorno = recv(socketClienteLfs, &paqueteLFS, sizeof(t_header), MSG_WAITALL);

	if(retorno == -1)
	{
	  pthread_mutex_lock(&mutex_socket);
	  socketClienteLfs = -1;
	  pthread_mutex_unlock(&mutex_socket);
	  return;
	}

	char *data = malloc(paqueteLFS.payload_size);
	recv(socketClienteLfs, data, paqueteLFS.payload_size, MSG_WAITALL);
	free(data);



	retardoLFS();
}

int enviar_drop_lfs( char *tabla ){

	t_header paquete;
	paquete.emisor = MEMORIA;
	paquete.tipo_mensaje = DROP;

	int retorno;
	verificarSocketLFS();

	char* buffer = serializar_string(tabla, &(paquete.payload_size));
	retorno = send(socketClienteLfs, &paquete, sizeof(t_header), 0);

	t_header paqueteLFS;

	if(retorno == -1)
	{
		free(buffer);
		log_error(mem_log, "LFS DEAD..."  );
		pthread_mutex_lock(&mutex_socket);
		socketClienteLfs = -1;
		pthread_mutex_unlock(&mutex_socket);
		paqueteLFS.tipo_mensaje = EJECUCIONERROR;
		return paqueteLFS.tipo_mensaje;
	}

	send(socketClienteLfs, buffer, paquete.payload_size, 0);
	retorno = recv(socketClienteLfs, &paqueteLFS, sizeof(t_header), MSG_WAITALL);

	if(retorno == -1)
	{
		free(buffer);
		log_error(mem_log, "LFS DEAD..."  );
		pthread_mutex_lock(&mutex_socket);
		socketClienteLfs = -1;
		pthread_mutex_unlock(&mutex_socket);
		paqueteLFS.tipo_mensaje = EJECUCIONERROR;
		return paqueteLFS.tipo_mensaje;
	}

	free(buffer);
	retardoLFS();

	return paqueteLFS.tipo_mensaje;
}

void enviar_create_lfs( linea_create linea_c ){

	t_header paquete;
	paquete.emisor = MEMORIA;
	paquete.tipo_mensaje = CREATE;

	int retorno;
	verificarSocketLFS();

	char* buffer = serializar_create(linea_c, &(paquete.payload_size));
	retorno = send(socketClienteLfs, &paquete, sizeof(t_header), 0);
	if(retorno == -1)
	{
		free(buffer);
		log_error(mem_log, "LFS DEAD..."  );
		pthread_mutex_lock(&mutex_socket);
		socketClienteLfs = -1;
		pthread_mutex_unlock(&mutex_socket);
		return;
	}

	send(socketClienteLfs, buffer, paquete.payload_size, 0);
	free(buffer);

	retardoLFS();

}


void journal()
{

	void journal_tabla_paginas(fila_TSegmentos *segmento){

		void journal_fila_paginas(fila_TPaginas* fila_pagina)
		{
			if(fila_pagina->modificado == 1){
			fila_Frames registro;
			leer_de_frame(fila_pagina->frame_registro,&registro);

			linea_insert linea;

			linea.tabla = segmento->nombre_tabla;
			linea.key = registro.key;
			linea.value = strdup(registro.value);

			enviar_insert_LFS(&linea);
			}
		}

		if( !list_is_empty( segmento->paginas ) ){
			list_iterate(segmento->paginas,(void*)journal_fila_paginas);
			list_destroy(segmento->paginas);
			log_info(mem_log, "LIBERADO TABLA DE PAGINAS");
		}
		else
		{
			list_destroy(segmento->paginas);
			log_info(mem_log, "LIBERADO TABLA DE PAGINAS");
		}

		free( segmento->nombre_tabla );//SE AGREGO AHORA
		free(segmento);
		log_info(mem_log, "LIBERADO SEGMENTO");

	}

	void journal_tabla_segmentos(t_list* tabla_segmentos) {

		if( !list_is_empty( tabla_segmentos ) ){
			list_iterate(tabla_segmentos,(void*)journal_tabla_paginas);
			list_clean(tabla_segmentos);
			log_info(mem_log, "LIMPIADO TABLA DE SEGMENTOS");
		}
	}

	journal_tabla_segmentos(tabla_segmentos);

	free(bitMapStr);
	bitarray_destroy(bitmap_frames);
	log_info(mem_log, "SE RESETEA EL BITMAP");

	bitMapStr = calloc(ceiling(cantidad_frames, 8), 1);
	bitmap_frames = bitarray_create_with_mode(bitMapStr, ceiling(cantidad_frames, 8), MSB_FIRST);

	frames_ocupados = 0;
}

void hilo_journal()
{
    struct timespec ts;
    ts.tv_sec = mem_config.retardo_journal / 1000;
    ts.tv_nsec = (mem_config.retardo_journal  % 1000) * 1000000;

    assignHandler();

	while ( !EXIT_PROGRAM ) {

	    nanosleep(&ts, NULL);
	    pthread_mutex_lock(&mutex);
	    journal();
	    pthread_mutex_unlock(&mutex);

	}

	pthread_exit(0);
}



void gossiping(){


	void gossiping_seed( t_memoria *memoria ){

		if( memoria->numero_memoria != mem_config.memory_number ){

			//verifico si esta desactivada para tratarme de conectar
			if( memoria->activa == 0 || memoria->socket == -1 ){

				int socketSeed = socket_connect_to_server(memoria->ip,  memoria->puerto );
				if( socketSeed == -1  ){

					memoria->socket=-1;
					log_error(mem_log, "¡Error no se pudo conectar con MEMORIA");
					return;
				}
				//si me conecto verifico si es una memoria del archivo de config (activa=0) para obtener el numero de memoria
				else{

					if( memoria->activa == 0 ){

						log_info(mem_log, "ME CONECTE CON UNA MEMORIA CON SOCKET:%d" ,socketSeed);
						t_header buffer;
						buffer.emisor=MEMORIA;
						buffer.tipo_mensaje =CONEXION;
						buffer.payload_size = 0;
						send(socketSeed, &buffer, sizeof( buffer ) , 0);
						log_info(mem_log, "HAGO EL SEND");
						int numero_memoria_seed;
						recv(socketSeed , &numero_memoria_seed, sizeof(int), MSG_WAITALL);
						log_info(mem_log, "HAGO EL RECV");
						memoria->socket=socketSeed;
						memoria->activa=1;
						memoria->numero_memoria = numero_memoria_seed;
						log_info(mem_log, "Se creo el socket con MEMORIA de numero:%d , de la memoria numero:%d", memoria->socket , memoria->numero_memoria);
					}
					else{
						memoria->socket=socketSeed;
						log_info(mem_log, "Se creo el socket con MEMORIA de numero:%d , de la memoria numero:%d", memoria->socket , memoria->numero_memoria);
					}
				}
			}


			log_info(mem_log, "MEMORIA gossiping activa numero:%d hago intercambio de tablas" ,memoria->numero_memoria );
			t_header buffer;
			buffer.emisor=MEMORIA;
			buffer.tipo_mensaje = GOSSIPING;

			/*ENVIO MEMORIAS ACTIVAS*/
			t_list* memorias_activas = get_memorias_activas( tabla_memorias );

			char* data = serializar_memorias(memorias_activas,&buffer.payload_size);
			int retorno = send(memoria->socket, &buffer, sizeof( buffer ) , 0);

			if (retorno == -1)
			{
				free(data);
				list_destroy(memorias_activas);
				log_error(mem_log, "Se murio MEMORIA de numero: %d , de la memoria numero:%d",memoria->numero_memoria);
				memoria->socket = -1;
				return;
			}

			send(memoria->socket, data, buffer.payload_size , 0);
			free(data);

			list_destroy(memorias_activas);

			t_header header_memoria;
			retorno = recv(memoria->socket , &header_memoria, sizeof(t_header), MSG_WAITALL);

			if (retorno == -1)
			{
				log_error(mem_log, "Se murio MEMORIA de numero: %d , de la memoria numero:%d",memoria->numero_memoria);
				memoria->socket = -1;
				return;
			}

			char *buffer_tabla = malloc( header_memoria.payload_size);
			recv(memoria->socket , buffer_tabla, header_memoria.payload_size , MSG_WAITALL);

			t_list *memorias_seed = deserializar_memorias(buffer_tabla);
			agregar_memorias_gossiping( memorias_seed );

			free( buffer_tabla );
			liberar_tabla_memorias(memorias_seed);

		}

	}

	if( !list_is_empty( tabla_memorias )  ){

		list_iterate( tabla_memorias  , (void*) gossiping_seed  );
	}

}



void agregar_memorias_gossiping( t_list *memorias_seed ){


	void agregar_memoria_gossip( pmemoria *memoria_seed ){


		bool memoria_encontrada( t_memoria *memoria_tabla ){

			if( memoria_tabla->numero_memoria == memoria_seed->numero_memoria ) return true;
			return false;
		}

		//si no la encuentra la agrego a la lista
		if( list_find( tabla_memorias , (void*)memoria_encontrada ) == NULL ){

			t_memoria* memoria_nueva = malloc( sizeof( t_memoria ) );
			memoria_nueva->activa=1;
			memoria_nueva->numero_memoria=memoria_seed->numero_memoria;
			memoria_nueva->ip = strdup(memoria_seed->ip);
			memoria_nueva->puerto = strdup(memoria_seed->puerto);
			memoria_nueva->socket=-1;
			list_add(tabla_memorias , memoria_nueva );
			log_info(mem_log, "se agrega al pool la memoria: %d",  memoria_nueva->numero_memoria );
		}
		else{
			log_info(mem_log, "ya se encuentra en el pool la memoria: %d",  memoria_seed->numero_memoria );
		}

	}

	//recorro lista de gossiping y agrego las nuevas
	list_iterate( memorias_seed , (void*)agregar_memoria_gossip );

}



t_list* get_memorias_activas( t_list* tabla_memorias ){

	bool is_memoria_activa( t_memoria* memoria ){

		return memoria->activa ? true : false;
	}

	return list_filter( tabla_memorias , (void*) is_memoria_activa  );
}


void hilo_gossiping(){

	struct timespec ts;
	ts.tv_sec = mem_config.retardo_gossiping / 1000;
	ts.tv_nsec = (mem_config.retardo_gossiping  % 1000) * 1000000;

	assignHandler();

	void logear_memoria( t_memoria *memoria ){

		log_info(mem_log, "MEMORIA: %d", memoria->numero_memoria );
	}

	while ( !EXIT_PROGRAM ) {

		nanosleep(&ts, NULL);

		log_info(mem_log, "INICIA gossiping" );
		list_iterate( tabla_memorias , (void*)logear_memoria );

		gossiping();

		log_info(mem_log, "FINALIZA gossiping" );

		//loggeo tabla seed
		list_iterate( tabla_memorias , (void*)logear_memoria );

	}

	pthread_exit(0);

}

void inotify_config(){

 	char buffer[BUF_LEN];

 	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		perror("inotify_init");
	}

 	int watch_descriptor = inotify_add_watch(file_descriptor, CONFIG_FOLDER, IN_MODIFY | IN_CREATE | IN_CLOSE_WRITE);

 	assignHandler();

 	while(!EXIT_PROGRAM){

 		int length = read(file_descriptor, buffer, BUF_LEN);
		if (length < 0) {
			perror("read");
		}

 		int offset = 0;

 		while (offset < length) {

 			struct inotify_event *event = (struct inotify_event *) &buffer[offset];

 			if (event->len) {
				//log_info(g_logger, "Event detected on: %s", event->name);
				if (string_contains(event->name, fileCFG)){
					log_info(mem_log, "Config File changed");

					config = config_create(rutaCFG);
					mem_config.retardo_mem = config_get_int_value(config, "RETARDO_MEM");
					mem_config.retardo_fs = config_get_int_value(config, "RETARDO_FS");
					mem_config.retardo_journal = config_get_int_value(config,"RETARDO_JOURNAL");
					mem_config.retardo_gossiping = config_get_int_value(config,"RETARDO_GOSSIPING");

					config_destroy(config);

					imprimir_config();
				}
			}
			offset += sizeof (struct inotify_event) + event->len;
		}

 	}

 	inotify_rm_watch(file_descriptor, watch_descriptor);
	close(file_descriptor);


 	pthread_exit(0);
}


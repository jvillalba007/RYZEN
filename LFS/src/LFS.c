#include "LFS.h"

void crear_servidor(){

	int cliente;

	log_info(g_logger, "El puerto de escucha es: %s", lfs_config.puerto_lfs);
	socketServidor = socket_create_listener( lfs_config.ip , lfs_config.puerto_lfs ); // @suppress("Symbol is not resolved")

	if( socketServidor < 0  ){

		log_error(g_logger, "¡Error no se pudo abrir el servidor ");
		exit(EXIT_FAILURE);
	}
	log_info(g_logger, "Se abre servidor de LFS");

	/* NUEVO CLIENTE */
	while( (cliente = socket_aceptar_conexion(socketServidor)) && (!EXIT_PROGRAM)  ){
		// INICIAR DETACHABLE
		pthread_t memoria_id;
		pthread_create(&memoria_id, NULL, (void*)funcionalidad_conexion_memoria, (void*)&cliente);
		pthread_t* idHilo = malloc(sizeof(pthread_t));
		*idHilo = memoria_id;
		log_info(g_logger, "[THREAD] Creo hilo para memoria");
		pthread_detach(memoria_id);
		list_add(threads,idHilo);
	}

	close(socketServidor);
	pthread_exit(0);

}

void liberar_threads() {
	void MatarHilos(pthread_t* id)
	{
		pthread_kill(*id, SIGUSR1);
	}
	list_iterate(threads, (void*) MatarHilos);
	list_destroy_and_destroy_elements(threads,free);
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
				liberar_threads();
			}
		else if ( strcmp(buffer, "") != 0){
			consola_procesar_comando(buffer);
		}

		free(buffer);
	}

	pthread_exit(0);

}

void handler(int id) {

}

void assignHandler() {
	struct sigaction sa = {0};
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
}

void hilo_compactacion(void* tabla)
{
	linea_create* metadata = (linea_create *) tabla;
    struct timespec ts;
    char* ctabla = strdup(metadata->tabla);
    u_int32_t compactacion_time = metadata->tiempo_compactacion;

    ts.tv_sec = compactacion_time / 1000;
    ts.tv_nsec = (compactacion_time  % 1000) * 1000000;

    assignHandler();

	while ( !EXIT_PROGRAM ) {

	    nanosleep(&ts, NULL);

	    if(compactate(ctabla))
	    break;

	}

	log_info(g_logger, "[THREAD] Se Cierra hilo de compactacion para %s",ctabla);

	liberar_linea_create(metadata);

	free(ctabla);
	pthread_exit(0);
}



void inotify_config(){

	char buffer[BUF_LEN];

	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		perror("inotify_init");
	}

	int watch_descriptor = inotify_add_watch(file_descriptor, CONFIG_FOLDER, IN_MODIFY | IN_CREATE | IN_CLOSE_WRITE);

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
				if (string_contains(event->name, CONFIG_FILE)){
					log_info(g_logger, "Config File changed");

					inotify_set_config();

					loggear_config();
				}
			}
			offset += sizeof (struct inotify_event) + event->len;
		}

	}

	inotify_rm_watch(file_descriptor, watch_descriptor);
	close(file_descriptor);


	pthread_exit(0);
}


void iniciar_hilos_compactacion()
{
	void hilos(linea_create* tabla)
	{
		// INICIAR DETACHABLE
		pthread_t thread_compactacion;
		pthread_create(&thread_compactacion, NULL, (void*)hilo_compactacion, (void*)tabla);
		pthread_t* idHilo = malloc(sizeof(pthread_t));
		*idHilo = thread_compactacion;
		log_info(g_logger, "[THREAD] Creo el hilo de compactacion para %s",tabla->tabla);
		pthread_detach(thread_compactacion);
		list_add(threads,idHilo);
	}

	t_list* tablas_metadata = procesar_describe(1, NULL);

	if(!list_is_empty(tablas_metadata))
	{
		list_iterate(tablas_metadata, (void*) hilos);
		list_destroy(tablas_metadata);
	}
	else
	{
		list_destroy(tablas_metadata);
	}

}


void liberar_general(){
	liberar_config(lfs_config);
	liberar_logger(g_logger);

	dictionary_destroy(table_status);
}

int main(void) {

	EXIT_PROGRAM = false;

	pthread_mutex_init(&mem_mutex, NULL);
	pthread_mutex_init(&isolation_mutex, NULL);
	pthread_mutex_init(&operation_mutex, NULL);

	iniciar_config();
	iniciar_montaje();
	iniciar_hilos_compactacion();

	// INICIAR HILO INOTIFY
	pthread_attr_t attr_inotify;
	pthread_attr_init(&attr_inotify);

	pthread_t tid_inotify;
	pthread_create(&tid_inotify, &attr_inotify, (void*)inotify_config, NULL);

	pthread_t* idHiloIno = malloc(sizeof(pthread_t));
	*idHiloIno = tid_inotify;
	list_add(threads,idHiloIno);

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

	// INICIAR DUMP
	pthread_attr_t attr_dump;
	pthread_attr_init(&attr_dump);

	pthread_t tid_dump;
	pthread_create(&tid_dump, &attr_dump, (void*)dump, NULL);
	pthread_t* idHilo = malloc(sizeof(pthread_t));
	*idHilo = tid_dump;
	list_add(threads,idHilo);


	//Esperar a que el hilo termine
	pthread_join(tid_consola, NULL);
	pthread_join(tid_dump, NULL);
	pthread_join(tid_server, NULL);
	pthread_join(tid_inotify, NULL);

	pthread_mutex_destroy(&mem_mutex);
	pthread_mutex_destroy(&isolation_mutex);
	pthread_mutex_destroy(&operation_mutex);

	liberar_memtable();
	liberar_bitmap();
	liberar_general();


	return 0;
}

void funcionalidad_conexion_memoria(void* clienteSocket){
	int cliente = *(int*) clienteSocket;

	log_info(g_logger,"HILO DE MEMORIA");
	t_header paquete;
	int result_recv;

	assignHandler();

	while((recv( cliente, &paquete, sizeof( t_header ) ,MSG_WAITALL)) && (!EXIT_PROGRAM)){

		log_info(g_logger, "[LFS] EVENTO: Emisor: MEMORIA, Tipo: %d, Tamanio: %d",paquete.tipo_mensaje,paquete.payload_size);

		char *buffer = NULL;

		if( paquete.payload_size > 0 ){

				buffer = malloc(paquete.payload_size);
				result_recv = recv(cliente, buffer, paquete.payload_size, MSG_WAITALL);
				if(result_recv == -1){
					free(buffer);
					break;
				}
		}


		switch(paquete.tipo_mensaje){

			case CONEXION:{
				send(cliente, &lfs_config.value_size,sizeof(int),MSG_NOSIGNAL);
			}
			break;

			case SELECT:{

				linea_select* lineas = malloc(sizeof(linea_select));
				deserializar_select(buffer,lineas);

				char* response;
				response = select_table_key(lineas);
				liberar_linea_select(lineas);

				t_header paquete;
				paquete.emisor = LFS;
				paquete.tipo_mensaje = response ? EJECUCIONOK : EJECUCIONERROR;
				paquete.payload_size = 0;

				if(paquete.tipo_mensaje == EJECUCIONOK)
				{
					char* serializado;

					linea_response_select linears;
					linears.value = response;
					linears.timestamp = 0;

					serializado = serializar_response_select(linears,&paquete.payload_size);
					send(cliente, &paquete, sizeof(t_header), MSG_NOSIGNAL);
					send(cliente, serializado, paquete.payload_size, MSG_NOSIGNAL);
					free(serializado);
					free(response);
				}
				else
				{
					send(cliente, &paquete,sizeof(t_header),MSG_NOSIGNAL);
				}
			}
			break;

			case INSERT:{

				linea_insert* lineas = malloc(sizeof(linea_insert));
				deserializar_insert(buffer,lineas);

				int response;
				response = insert_record(lineas, NULL);
				liberar_linea_insert(lineas);

				if (response==1)
				{
					log_error(g_logger, "No existe la tabla %s especificada. \n", lineas->tabla);
				}
				else if (response==2)
				{
					log_error(g_logger, "El value %s es demasiado grande. \n", lineas->value);
				}
			}
			break;

			case CREATE:{

				linea_create* lineas = malloc(sizeof(linea_create));
				deserializar_create(buffer,lineas);

				int response;
				char* particiones = string_from_format("%"PRIu8, lineas->nro_particiones);
				char* tiempoCompactacion = string_from_format("%"PRIu32, lineas->tiempo_compactacion);
				response = create_table(lineas->tabla, lineas->tipo_consistencia,particiones , tiempoCompactacion);

				
				t_header paquete;
				paquete.emisor = LFS;
				paquete.tipo_mensaje = EJECUCIONOK;
				paquete.payload_size = 0;

				if (response == 1)
				{
					paquete.tipo_mensaje = EJECUCIONERROR;
					log_error(g_logger, "Operación create %s no pudo ser realizada. Revise logs.\n", lineas->tabla);
				}

				send(cliente, &paquete,sizeof(t_header),MSG_NOSIGNAL);

				
			}
			break;

			case DESCRIBE:{
				char* describe;

				if(paquete.payload_size > 0)
				{
					char* tabla = deserializar_string(buffer);
					describe = string_from_format("DESCRIBE %s", tabla);

					free(tabla);
				}
				else
				{
					describe = strdup("DESCRIBE");
				}

				log_info(g_logger, "%s \n",describe);

				char** parametros = string_split(describe, " ");
				int cant_parametros = split_cant_elem(parametros);

				t_list* definitiva = list_create();

				void* response = procesar_describe(cant_parametros, parametros);

				t_header paquete;
				paquete.emisor = LFS;
				paquete.tipo_mensaje = response ? EJECUCIONOK : EJECUCIONERROR;
				paquete.payload_size = 0;

				if (cant_parametros == 1){
					(list_is_empty(response)) ? list_destroy(response) : list_add_all(definitiva,response);
				}
				else if(response != NULL)
				{
						list_add(definitiva,response);
				}

				if(paquete.tipo_mensaje == EJECUCIONERROR || list_is_empty(definitiva))
				{
					paquete.tipo_mensaje = EJECUCIONERROR;
					log_error(g_logger, "Operación DESCRIBE %s no pudo ser realizada. Revise logs.\n",parametros[1]);
					send(cliente, &paquete,sizeof(t_header),MSG_NOSIGNAL);

					list_destroy(definitiva);
					free(describe);
					split_liberar(parametros);
					break;
				}

				char* serializado;
				serializado = serializar_describe(definitiva,&paquete.payload_size);
				send(cliente, &paquete, sizeof(t_header), MSG_NOSIGNAL);
				send(cliente, serializado, paquete.payload_size, MSG_NOSIGNAL);

				free(serializado);
				list_destroy(definitiva);
				free(describe);
				split_liberar(parametros);

			}
			break;

			case DROP:{

				char* tabla = deserializar_string(buffer);

				int response;
				response = drop_table(tabla);

				t_header paquete;
				paquete.emisor = LFS;
				paquete.tipo_mensaje = EJECUCIONOK;
				paquete.payload_size = 0;

				if (response == 1)
				{
					paquete.tipo_mensaje = EJECUCIONERROR;
					log_error(g_logger, "La tabla %s especificada no existe. \n", tabla);
				}

				send(cliente, &paquete,sizeof(t_header),MSG_NOSIGNAL);
				free(tabla);
			}
			break;

			if (buffer != NULL)
			free(buffer);
		}
	}
	log_info(g_logger,"SE CIERRA HILO CONEXION\n");
	close(cliente);
	pthread_exit(0);

}


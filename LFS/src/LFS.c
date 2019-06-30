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
		else {
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

	free(ctabla);
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
		list_destroy_and_destroy_elements(tablas_metadata, (void*) liberar_linea_create);
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

	iniciar_config();
	iniciar_montaje();
	iniciar_hilos_compactacion();

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

	liberar_memtable();
	liberar_bitmap();
	liberar_general();


	return 0;
}

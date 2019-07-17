/*
 * commons.c
 *
 *  Created on: 9 jun. 2019
 *      Author: utnso
 */


#include "commons.h"




void inicializar_logs_y_configs() {
    abrir_log();
    crear_config();
	leer_configs();
    loggear_inicio_logger();
	loggear_configs();
}

void abrir_log() {
    logger = log_create("kernel.log", "KERNEL", 0 , LOG_LEVEL_INFO);
}

void crear_config() {
	config = config_create("config/kernel.cfg");
}

void leer_configs() {
	kernel_config.IP_MEMORIA = strdup(config_get_string_value(config, "IP_MEMORIA"));
	kernel_config.PUERTO_MEMORIA = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));
	kernel_config.MULTIPROCESAMIENTO = config_get_int_value(config, "MULTIPROCESAMIENTO");
	kernel_config.SLEEP_EJECUCION = config_get_int_value(config, "SLEEP_EJECUCION");
	kernel_config.METADATA_REFRESH = config_get_int_value(config, "METADATA_REFRESH");
	kernel_config.GOSSIPING_REFRESH = config_get_int_value(config, "GOSSIPING_REFRESH");
    kernel_config.QUANTUM = config_get_int_value(config, "QUANTUM");

	//	Clausura de la estructura config
	config_destroy(config);
}

void loggear_inicio_logger() {
    log_info(logger, "Kernel > Inicio del Logger");
}

void loggear_configs() {
    log_info(logger, "IP_MEMORIA: %s", kernel_config.IP_MEMORIA);
    log_info(logger, "PUERTO_MEMORIA: %s", kernel_config.PUERTO_MEMORIA);
    log_info(logger, "MULTIPROCESAMIENTO: %d", kernel_config.MULTIPROCESAMIENTO);
    log_info(logger, "SLEEP_EJECUCION: %d", kernel_config.SLEEP_EJECUCION);
    log_info(logger, "METADATA_REFRESH: %d", kernel_config.METADATA_REFRESH);
    log_info(logger, "QUANTUM: %d", kernel_config.QUANTUM);
}



t_list* get_memorias_activas( t_list* tabla_memorias ){

	bool is_memoria_activa( t_memoria_del_pool * memoria ){

		return memoria->activa ? true : false;
	}

	return list_filter( tabla_memorias , (void*) is_memoria_activa  );
}


t_memoria_del_pool* obtener_memoria_random( t_list *memorias ){

	if (list_is_empty(memorias)) return NULL;

	t_list* memorias_activas = get_memorias_activas(memorias) ;
	t_memoria_del_pool* mem = NULL;

	if( list_is_empty( memorias_activas )) {

		list_destroy( memorias_activas);
		return NULL;
	}

	int index = rand_num(list_size( memorias_activas));
	mem = list_get( memorias_activas, index);

	list_destroy( memorias_activas);
	return mem;
}

void retardo(){

	struct timespec ts;
	ts.tv_sec = kernel_config.SLEEP_EJECUCION / 1000;
	ts.tv_nsec = (kernel_config.SLEEP_EJECUCION  % 1000) * 1000000;

	log_info(logger, "Durmiendo por %d milisegundos" , kernel_config.SLEEP_EJECUCION);
	nanosleep(&ts, NULL);
}


int rand_num(int max){
	int numero;
	numero = rand() % max;

	return numero;
}

void liberar_config() {
    free(kernel_config.IP_MEMORIA);
    free(kernel_config.PUERTO_MEMORIA);
}


void liberar_kernel(){

	log_info(logger, "libera hilos procesadores");
	list_destroy_and_destroy_elements(l_procesadores  , (void*)free );

	log_info(logger, "libero semaforo");
	pthread_mutex_destroy(&sem_ejecutar);

	log_info(logger, "libera lista criterios");
	//FIN lista criterios
	list_destroy(l_criterio_SHC);
	list_destroy(l_criterio_SC);
	list_destroy(l_criterio_EC);

	log_info(logger, "libera lista tablas");
	list_destroy_and_destroy_elements(l_tablas , (void*)free_tabla);

	log_info(logger, "libera lista memorias");
	list_destroy_and_destroy_elements(l_memorias , (void*)free_memoria);

	//FIN lista de estados
	log_info(logger, "libera lista estados");
	list_destroy_and_destroy_elements(l_pcb_nuevos, (void*)free_Pcb);
	list_destroy_and_destroy_elements(l_pcb_listos, (void*)free_Pcb);
	list_destroy_and_destroy_elements(l_pcb_ejecutando, (void*)free_Pcb);
	list_destroy_and_destroy_elements(l_pcb_finalizados, (void*)free_Pcb);

	/*log_info(logger, "libera hilos procesadores");
	terminar_hilos_procesadores();
	list_destroy_and_destroy_elements(l_procesadores  , (void*)free );*/

	log_info(logger, "libera config");
	liberar_config();
	log_destroy(logger);
}

void free_Pcb(t_PCB* pcb){
	log_info(logger, "libera pcb id:%d" , pcb->id );
	free(pcb->request_comando);
	free(pcb);
}

void free_memoria(t_memoria_del_pool* memoria_borrar){

	log_info(logger, "libera memoria:%d" , memoria_borrar->numero_memoria );
	free(memoria_borrar->criterio);
	free(memoria_borrar->ip);
	free(memoria_borrar->puerto);
	free(memoria_borrar);
}

void free_tabla(t_tabla_consistencia* tabla_borrar){

	log_info(logger, "libera tabla:%s" , tabla_borrar->nombre_tabla );
	free(tabla_borrar->nombre_tabla);
	free(tabla_borrar->criterio_consistencia);
	free(tabla_borrar);
}

void liberar_memorias_gossiping(t_list *memorias){

	list_destroy_and_destroy_elements(memorias , (void*)free_memoria_gossiping);
}

void free_memoria_gossiping( pmemoria *memoria ){

	free( memoria->ip );
	free( memoria->puerto );
	free( memoria );
}

void free_tabla_describe( linea_create *linea ){

	free(linea->tabla);
	free(linea->tipo_consistencia);
	free(linea);
}


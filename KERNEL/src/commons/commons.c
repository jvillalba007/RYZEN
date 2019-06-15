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
    config = config_create("../kernel.cfg");
}

void leer_configs() {
	kernel_config.IP_MEMORIA = strdup(config_get_string_value(config, "IP_MEMORIA"));
	kernel_config.PUERTO_MEMORIA = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));
	kernel_config.MULTIPROCESAMIENTO = config_get_int_value(config, "MULTIPROCESAMIENTO");
	kernel_config.SLEEP_EJECUCION = config_get_int_value(config, "SLEEP_EJECUCION");
	kernel_config.METADATA_REFRESH = config_get_int_value(config, "METADATA_REFRESH");
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

void liberar_config() {
    free(kernel_config.IP_MEMORIA);
    free(kernel_config.PUERTO_MEMORIA);
}


void liberar_kernel(){

	log_info(logger, "libero semaforo");
	pthread_mutex_destroy(&sem_ejecutar);

	log_info(logger, "libera lista criterios");
	//FIN lista criterios
	list_destroy(l_criterio_SHC);
	list_destroy(l_criterio_SC);
	list_destroy(l_criterio_EC);

	log_info(logger, "libera lista tablas");
	list_destroy_and_destroy_elements(l_tablas , free_memoria);

	log_info(logger, "libera lista memorias");
	list_destroy_and_destroy_elements(l_memorias , free_memoria);

	//FIN lista de estados
	log_info(logger, "libera lista estados");
	list_destroy_and_destroy_elements(l_pcb_nuevos, free_Pcb);
	list_destroy_and_destroy_elements(l_pcb_listos, free_Pcb);
	list_destroy_and_destroy_elements(l_pcb_ejecutando, free_Pcb);
	list_destroy_and_destroy_elements(l_pcb_finalizados, free_Pcb);

	log_info(logger, "libera hilos procesadores");
	terminar_hilos_procesadores();
	list_destroy(l_procesadores);

	log_info(logger, "libera config");
	liberar_config();
	log_destroy(logger);
}

void free_Pcb(void* pcb_borrar){
	t_PCB* pcb = (t_PCB*) pcb_borrar;
	free(pcb->request_comando);
	free(pcb);
}

void free_memoria(void* memoria_borrar){


}

void free_tabla(void* tabla_borrar){


}

void terminar_hilos_procesadores(){

	void terminar_hilo_procesador(pthread_t* id_hilo)
	{
		log_info(logger, "cierro hilo de ejecucion id :%d" , *id_hilo);
		//pthread_cleanup_push( id_hilo );
		pthread_cancel( *id_hilo);
	}

	list_iterate(l_procesadores,(void*)terminar_hilo_procesador);

}





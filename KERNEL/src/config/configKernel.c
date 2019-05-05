#include "configKernel.h"

void inicializar_logs_y_configs() {
    abrir_log();
    crear_config();
	leer_configs();
    loggear_inicio_logger();
	loggear_configs();
}

void abrir_log() { 
    logger = log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_INFO);
}

void crear_config() {
    config = config_create("kernel.cfg");
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
    log_info(logger, "KERNEL - Inicio del Logger");
}

void loggear_configs() {
    log_info(logger, "IP_MEMORIA: %s", kernel_config.IP_MEMORIA);
    log_info(logger, "PUERTO_MEMORIA: %s", kernel_config.PUERTO_MEMORIA);
    log_info(logger, "MULTIPROCESAMIENTO: %d", kernel_config.MULTIPROCESAMIENTO);
    log_info(logger, "SLEEP_EJECUCION: %d", kernel_config.SLEEP_EJECUCION);
    log_info(logger, "METADATA_REFRESH: %d", kernel_config.METADATA_REFRESH);
    log_info(logger, "QUANTUM: %d", kernel_config.QUANTUM);
}

void liberar_kernel_config(ck kernel_config) {
    free(kernel_config.IP_MEMORIA);
    free(kernel_config.PUERTO_MEMORIA);
}
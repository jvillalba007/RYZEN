#include "config_LFS.h"

void iniciar_logger(void)
{

	g_logger = log_create("LFS.log", "LFS", 0, LOG_LEVEL_INFO);

	log_info(g_logger, "logger iniciado");
}

void crear_config() {
    config = config_create("LFS.cfg");
}

void leer_config() {
	lfs_config.puerto_lfs = strdup(config_get_string_value(config, "PUERTO_LFS"));
	lfs_config.punto_montaje = strdup(config_get_string_value(config, "PUNTO_MONTAJE"));
	lfs_config.retardo_lfs = config_get_int_value(config, "RETARDO_LFS");
	lfs_config.value_size = config_get_int_value(config, "VALUE_SIZE");
	lfs_config.tiempo_dump = config_get_int_value(config, "TIEMPO_DUMP");

	//	Clausura de la estructura config
	config_destroy(config);
}

void loggear_config() {
    log_info(g_logger, "PUERTO LFS: %s", lfs_config.puerto_lfs);
    log_info(g_logger, "PUNTO MONTAJE: %s", lfs_config.punto_montaje);
    log_info(g_logger, "RETARDO LFS: %d", lfs_config.retardo_lfs);
    log_info(g_logger, "TAMANO DE VALUE: %d", lfs_config.value_size);
    log_info(g_logger, "TIEMPO DE DUMP: %d", lfs_config.tiempo_dump);
}

void liberar_config(lfs_cfg lfs_config) {
    free(lfs_config.puerto_lfs);
    free(lfs_config.punto_montaje);
}

void liberar_logger(t_log* g_logger){
	log_destroy(g_logger);
}

void iniciar_config(){
	iniciar_logger();
	crear_config();
	leer_config();
	loggear_config();
}

#ifndef CONFIG_LFS_H
	#define CONFIG_LFS_H

	#include <stdio.h>
	#include <string.h>
	#include<commons/log.h>
	#include <commons/config.h>

	typedef struct {
		char* puerto_lfs;
		char* punto_montaje;
		int retardo_lfs;
		int value_size;
		int tiempo_dump;
	} lfs_cfg;

	t_log* g_logger;
	t_config* config;
	lfs_cfg lfs_config;

	void iniciar_logger(void);
	void iniciar_config(void);
	void crear_config(void);
	void leer_config(void);
	void loggear_config(void);
	void liberar_config(lfs_cfg);

#endif

#ifndef CONFIG_LFS_H
	#define CONFIG_LFS_H
	#define ceiling(x,y) (((x) + (y) - 1) / (y))

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <commons/log.h>
	#include <commons/config.h>
	#include <commons/string.h>
	#include <commons/collections/list.h>
	#include <shared/utils.h>

	typedef struct {
		char* puerto_lfs;
		char* punto_montaje;
		int retardo_lfs;
		int value_size;
		int tiempo_dump;
	} lfs_cfg;

	t_log* g_logger;
	t_config* config;
	t_config* metadata;
	lfs_cfg lfs_config;

	t_list* memtable;

	char* BLOCK_SIZE;
	char* BLOCKS;
	char* MAGIC_NUMBER;

	void iniciar_logger(void);
	void iniciar_config(void);
	void crear_config(void);
	void leer_config(void);
	void loggear_config(void);
	void liberar_logger(t_log*);
	void liberar_config(lfs_cfg);
	void iniciar_memtable();
	void iniciar_bitmap();
	void iniciar_montaje(void);
	void crear_metadata(void);
	void leer_metadata(void);
	void loggear_metadata(void);
	//void crear_carpeta(char* carpeta);

#endif

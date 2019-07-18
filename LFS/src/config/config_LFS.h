#ifndef CONFIG_LFS_H
	#define CONFIG_LFS_H
	#define ceiling(x,y) (((x) + (y) - 1) / (y))

	#define max(a,b) \
		({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; })

	#define min(a,b) \
		({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a < _b ? _a : _b; })


	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/mman.h>
	#include <sys/stat.h>
	#include <commons/log.h>
	#include <commons/config.h>
	#include <commons/string.h>
	#include <commons/collections/list.h>
	#include <shared/utils.h>
	#include <commons/bitarray.h>
	#include <errno.h>
	#include <inttypes.h>
	#include <sys/inotify.h>

	#include "shared/protocolo.h"

	#define CONFIG_FILE "LFS.cfg"

	#define TABLES_FOLDER "tables/"
	#define EVENT_SIZE ( sizeof (struct inotify_event) + 8 )
	#define BUF_LEN ( 1024 * EVENT_SIZE )

	typedef struct {
		char* puerto_lfs;
		char* punto_montaje;
		char* ip;
		int retardo_lfs;
		int value_size;
		int tiempo_dump;
	} lfs_cfg;

	t_log* g_logger;
	t_config* config;
	t_config* metadata;
	lfs_cfg lfs_config;

	t_list* memtable;
	t_list* threads;
	t_bitarray* bitmap;

	t_dictionary* table_status; // FALSE means CAN USE - TRUE means CAN'T USE (BLOCKED)

	char* BLOCK_SIZE;
	char* BLOCKS;
	char* MAGIC_NUMBER;

	char APP_DIR[PATH_MAX];
	char* CONFIG_PATH;
	char* CONFIG_FOLDER;

	void iniciar_logger(void);
	void iniciar_config(void);
	void leer_config(void);
	void loggear_config(void);
	void liberar_logger(t_log*);
	void liberar_config(lfs_cfg);
	void liberar_bitmap();
	void iniciar_memtable();
	void iniciar_bitmap();
	void iniciar_montaje(void);
	void crear_metadata(void);
	void leer_metadata(void);
	void loggear_metadata(void);
	void iniciar_dict_table_status(void);
	void setear_ruta_config(void);
	void inotify_set_config(void);

	//void crear_carpeta(char* carpeta);

#endif

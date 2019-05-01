#ifndef LFS_H
	#define LFS_H
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <commons/log.h>
	#include <commons/config.h>
	#include <commons/string.h>
	#include <pthread.h>
	#include <shared/socket.h>
	#include <shared/console.h>

	#include "shared/utils.h"
	#include "config/config_LFS.h"
	#include "API.h"

	bool EXIT_PROGRAM;
	int socketServidor;

	void crear_servidor(void);
	void console_process(void);
	void liberar_memoria(void);
	void consola_procesar_comando(char*);

#endif /* LFS_H */



#ifndef LFS_H
	#define LFS_H
	#include <stdio.h>
	#include <string.h>
	#include<commons/log.h>
	#include <commons/config.h>
	#include <pthread.h>
	#include <shared/socket.h>
	#include <shared/console.h>

	#include "config/config_LFS.h"

#endif /* LFS_H */


int socketServidor;

void crear_servidor(void);
void iniciar_logger(void);
void console_process(size_t);
void liberar_memoria(void);

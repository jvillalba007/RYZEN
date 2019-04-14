#ifndef LFS_H
	#define LFS_H
	#include <stdio.h>
	#include <stdlib.h>
	#include<commons/log.h>
	#include <pthread.h>
	#include <shared/socket.h>
	#include <shared/console.h>

#endif /* LFS_H */

size_t bufsize = 32;
t_log* g_logger;

void iniciar_logger(void);

void console_process(size_t);

void liberar_memoria(void);

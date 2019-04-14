#include "LFS.h"




void iniciar_logger(void)
{

	g_logger = log_create("LFS.log", "LFS", 1, LOG_LEVEL_INFO);

	log_info(g_logger, "logger iniciado");
}



void console_process(size_t bufsize) {

	bool exit_loop = true;

	while (exit_loop) {

		char* buffer;
		size_t n_characters;

		buffer = (char*) malloc(bufsize * sizeof(char));

		n_characters = console(&buffer);
		log_info(g_logger, buffer);

		if ( 0 == strcmp(buffer, "exit\n") ) exit_loop = false;
		free(buffer);

	}

	pthread_exit(0);
}

void liberar_memoria(){
	log_destroy(g_logger);
}


int main(void) {

	iniciar_logger();

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_t tid;

	pthread_create(&tid, &attr, console_process, NULL);


	//Esperar a que el hilo termine
	pthread_join(tid, NULL);

	liberar_memoria();

}

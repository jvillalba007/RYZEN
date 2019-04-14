#include "LFS.h"


size_t bufsize = 32;

void console_process(size_t bufsize) {

	bool exit_loop = true;

	while (exit_loop) {

		char* buffer;
		size_t n_characters;

		buffer = (char*) malloc(bufsize * sizeof(char));

		n_characters = console(&buffer);
		printf("You typed: '%s'\n", buffer);

		if ( 0 == strcmp(buffer, "exit\n") ) exit_loop = false;
		free(buffer);

	}

	pthread_exit(0);
}

int main(void) {

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_t tid;

	pthread_create(&tid, &attr, console_process, NULL);


	//Esperar a que el hilo termine
	pthread_join(tid, NULL);

}

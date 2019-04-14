#ifndef CONSOLE_SHARED_H
#define CONSOLE_SHARED_H

	#include <stdio.h>
	#include <stdlib.h>


	/**
	* @NAME: console
	* @DESC: /* Recibe un buffer alocado con los chars necesarios para almacenar el mensaje. Lee por console el mensaje y lo mete
	* en la variable buffer. Retorna el tipo de dato size_t que dice la cantidad de caracteres que recibió, contando \0.
	* No olvidarse de freerear el buffer una vez procesado.
	* Ejemplo:
	int main(void){
		char *buffer;
		size_t n_characters;

		buffer = (char *)malloc(bufsize * sizeof(char));

		n_characters = console(&buffer);

		printf("You typed: '%s'\n", buffer);

		free(buffer);
	}

	*/

	size_t console(char *buffer);


#endif /* SHARED_SOCKET_H_ */

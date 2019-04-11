/*
 ============================================================================
 Name        : MEMORIA.c
 Author      : RYZEN
 Version     :
 Copyright   : 2019
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <shared/socket.h>

int main(void) {
	puts("MEMORIA"); /* prints MEMORIA */

	//prueba funcion libreria compartida funcion max
	printf("%d\n",max(3,10));

	return EXIT_SUCCESS;
}

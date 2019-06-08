#ifndef UTILS_SHARED_H
#define UTILS_SHARED_H

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>

	// Libera memoria de un array de chars recorriendo 1 por 1
	void split_liberar(char **);

	// Cuenta la cantidad de strings que hay en un array
	int split_cant_elem(char **);
	
	/* Recibe el string a procesar, el string-patrón con el que
	 * debería empezar y otro con el que debería terminar.
	 * Devuelve el string que extrajo en el medio.
	 * ADVERTENCIA: LIBERAR ESE STRING DESPUES DE USAR.
	*/
	char* string_extract_substring(char*, char*, char*);
	
	/* Recibe 2 strings: el primero es la linea a procesar y el segundo
	 * es lo que hay que quitar de ese primer string.
	 * Ejemplo: remove_substring("hola como estas", "como") => "hola  estas"
	 * Liberar memoria de los strings si es necesario.
	 */
	void remove_substring (char *, char*);

	/* Recibe 2 strings: el primero es la linea a procesar y el segundo
	 * es value (sin comillas) que hay que quitar de esa linea.
	 * Ejemplo: remove_value("insert tabla1 1 "a1" timestamp","a1") => "insert tabla1 1 timestamp"
	 * Ya esta lindo y agradable para splitear linea con un espacio!
	 */
	void remove_value (char *, char*);

	/* Recibe como parámetro un string.
	 * Si el último caracter es un '\n', se elimina.
	 * Si no lo es, no pasa nada.
	 */
	void limpiar_caracter_final_de_nueva_linea (char *);

	/* Recibe como parametro el nombre de la carpeta, el nombre del archivo
	 * y la extensión. Devuelve un puntero al string concatenado.
	 * ADVERTENCIA: LIBERAR ESE STRING DESPUES DE USAR.
	 */
	char* generate_path(char*, char*, char*);

	/* Si el string es un número devuelve 1, sino 0.*/
	int isNumeric (const char*);

#endif /* SHARED_SOCKET_H_ */

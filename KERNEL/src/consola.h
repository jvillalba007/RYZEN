#ifndef CONSOLA_H
#define CONSOLA_H

	#include <shared/console.h>
	#include "shared/utils.h"
	#include <commons/string.h>
	#include <pthread.h>
	#include "config/configKernel.h"

//  Estructura que almacenará, para cada tabla existente, lo siguiente: nombre de tabla - criterio de consistencia.
typedef struct {
    char* nombre_tabla;
    char* criterio_consistencia;
    //puntero a siguiente
} struct_todas_las_tablas;

//  Estructura que almacenará, para cada memoria existente en el pool de memorias, lo siguiente: número de memoria - ip - puerto.
typedef struct {
    int numero_memoria;
    char* ip;
    char* puerto;
    //puntero a siguiente
} struct_todas_las_memorias;

//  "Estructura" que almacenará el número de memoria correspondiente a la única memoria con criterio de consistencia SC.
int numero_memoria_con_criterio_SC;

//  Estructura que almacenará todos los números de memorias que tengan el criterio de consistencia SHC. 
typedef struct {
    int numero_memoria;
    //puntero a siguiente
} struct_memorias_SHC;

//  Estructura que almacenará todos los números de memorias que tengan el criterio de consistencia EC.
typedef struct {
    int numero_memoria;
    //puntero a siguiente
} struct_memorias_EC;

void consola();
void procesar_comando(char*);
void notificar_error_sintactico_en_comando(void);
void notificar_error_sintactico_en_parametros(void);
void notificar_error_tipo_consistencia(void);

#endif
#ifndef KERNEL_H
#define KERNEL_H

#include <pthread.h>
#include "commons/commons.h"
#include "consola.h"
#include <shared/socket.h>
#include <shared/console.h>
#include <shared/protocolo.h>
#include "shared/utils.h"

void conectar_memoria();
void inicializar_kernel(); //inicializa las listas que se utilizaran en kernel
void ejecutar_procesador(); //ejecuta en un hilo los pcb

t_PCB* obtener_pcb_ejecutar(); // obtiene un pcb a ejecutar de la listo de ready.
int ejecutar_linea( char *linea ); //recibe una linea y la envia a ejecutar a alguna memoria
char* obtener_nombre_tabla(  char** parametros ); //me devuelve el nombre de la tabla a partir de una linea de request. si es un describe sin tabla devuelve null
t_tabla_consistencia *obtener_tabla( char* n_tabla); //devuelve la tabla a partir del nombre
t_memoria_del_pool *obtener_memoria_criterio( t_tabla_consistencia* tabla, char* linea); //a partir de una tabla me indica que memoria va a realizar la request segun criterio
t_memoria_del_pool *obtener_memoria_SC();
t_memoria_del_pool *obtener_memoria_EC();
t_memoria_del_pool *obtener_memoria_SHC(char* linea);
int ejecutar_linea_memoria( t_memoria_del_pool* memoria , char* linea ); //ejecuta en memoria la linea

void crear_procesadores();
void ejecutar_describe();
char* obtener_linea(FILE* archivo);
void apuntar_archivo(FILE* archivo, int pc);

void finalizar_pcb(t_PCB* pcb);//quita pcb de ejecucion y lo pasa a finalizados
void parar_por_quantum(t_PCB* pcb);//quita pcb de ejecucion y lo pasa a listos
int rand_num(int max);

char *convertir_insert(char** split);
char *convertir_select(char** spit);
char *convertir_create(char** split);



#endif

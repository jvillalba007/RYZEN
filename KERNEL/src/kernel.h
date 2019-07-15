#ifndef KERNEL_H
#define KERNEL_H

#include <pthread.h>
#include "commons/commons.h"
#include "consola.h"
#include <shared/socket.h>
#include <shared/console.h>
#include <shared/protocolo.h>
#include "shared/utils.h"


pthread_t tid_estadisticas;
pthread_t tid_gossiping;
pthread_t tid_describe;
pthread_t tid_inotify;

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
void reinicio_estadisticas();

int enviar_insert(linea_insert linea, void* socket);
int enviar_select(linea_select linea, void* socket);
int enviar_create(linea_create linea, void* socket);
int enviar_describe_general(void* socket);
int enviar_describe_especial(void* socket, char* tabla);
int enviar_drop(void* socket, char* tabla);
void recibir_agregar_memoria(void* socket_memoria);

void hilo_gossiping(); //Ejecuta cada x tiempo el gossiping con alguna memoria
int gossiping( t_memoria_del_pool *memoria );
void agregar_memoria_gossip( pmemoria *memoria ); //verifica si la memoria recibida existe o no en la tabla para agregarla o desecharla

void agregar_tabla_describe( linea_create* tabla_describe ); //agrega tabla en el sistema si no existe

void hilo_describe();
int describe();

void quitar_tabla_lista( char* tabla );//a partir de un nombre de tabla la busca en la metadata y la borra si existe
void desactivar_memoria(t_memoria_del_pool *memoria); //desactiva la memoria por una desconexion. la quita de los criterios donde esta asociada.

void inotify_config();

#endif

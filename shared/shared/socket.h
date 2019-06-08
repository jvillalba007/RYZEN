#ifndef SHARED_SOCKET_H_
#define SHARED_SOCKET_H_
	#include <stdio.h>
	#include <stdbool.h>
	#include <string.h>
	#include <stdlib.h>
	#include <stdarg.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <commons/collections/list.h>

	typedef enum {DESCONOCIDO, KERNEL,MEMORIA,LFS} e_emisor;
	typedef enum {CONEXION,DESCONEXION,SELECT,INSERT,CREATE,DESCRIBE,DROP,JOURNAL} e_tipo_msg;

	typedef struct {
		e_emisor emisor;
		e_tipo_msg tipo_mensaje;
		int payload_size;
	} __attribute__((packed)) t_header;

	typedef struct {
		t_header* header;
		void* payload;
	} t_msg;
	
	typedef struct {
		int socket;
		e_emisor conectado;
		__SOCKADDR_ARG addr;
	} t_conexion;

	#define BACKLOG 100

	#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a > _b ? _a : _b; })

	int socket_a_destruir;

	/**
	* @NAME: msg_await
	* @DESC: Espera hasta recibir un mensaje. El socket ya debe estar escuchando
	*/
	int msg_await(int socket, t_msg* mensaje);

	/**
	* @NAME: msg_free
	* @DESC: Libera la memoria de un mensaje
	*/
	void msg_free(t_msg** msg);

	/**
	* @NAME: socket_create_listener
	* @DESC: Creo un socket de escucha y lo devuelvo, o -1 se hubo error. Me pongo a escuchar.
	* @PARAMS:
	* 		ip   - ip del server. Si es NULL, usa el localhost: 127.0.0.1
	* 		port - puerto en el que escuchar
	*/
	int socket_create_listener(char* ip, char* port);

	/**
	* @NAME: socket_connect_to_server
	* @DESC: Me conecto al server, y devuelvo el socket, o -1 si hubo error
	*/
	int socket_connect_to_server(char* ip, char* port);

	/**
	* @NAME: socket_aceptar_conexion
	* @DESC: Acepta una nueva conexión y devuelve el nuevo socket conectado
	*/
	int socket_aceptar_conexion(int socket_servidor);

	/**
	* @NAME: socket_get_ip
	* @DESC: Devuelve la IP de un socket
	*/
	char* socket_get_ip(int fd);

	/**
	* @NAME: socket_start_listening_select
	* @DESC: Gestiona eventos en un socket con la funcion que es parametro + en los FD adicionales
	* @PARAMS:
	* 	socketListener -
	* 	manejadorDeEvento - funcion a llamar en el caso de un evento con un descriptor de archivo
	* 	... - FD adicionales a multiplexar. Al principio, se debe indicar la cantidad de FDs, y para c/u su emisor y el tipo de msj.
	* 	      Ejemplos:
	* 		      1, SAFA, INOTIFY, fd_inotify
	* 		      2, SAFA, INOTIFY, fd_inotify1, SAFA, INOTIFY, fd_inotify2
	*/
	void socket_start_listening_select(int socketListener, int (*manejadorDeEvento)(int, t_msg*), ...);

	/*
	 * @NAME: enviarMensaje
	 * @DESC: Envia un mensaje al socketReceptor. Devuelve 0 si el envio fue exitoso,
	 * o -1 si no se enviaron todos los bytes.
	 */
	int enviar_socket(int socketReceptor, void *mensaje, int largoMensaje);
#endif /* SHARED_SOCKET_H_ */

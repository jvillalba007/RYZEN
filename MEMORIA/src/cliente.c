/*
 * cliente.c
 *
 *  Created on: 19 abr. 2019
 *      Author: utnso
 */

#include <shared/socket.h>

int main2(void){

	int socket = socket_connect_to_server("127.0.0.1",  "8000" );

	t_header buffer;
	buffer.emisor=KERNEL;
	buffer.tipo_mensaje = CONEXION ;
	buffer.payload_size = 32;

	send(socket, &buffer, sizeof( buffer ) , 0);

	return 0;
}

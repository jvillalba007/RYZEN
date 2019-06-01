/*
 * cliente.c
 *
 *  Created on: 19 abr. 2019
 *      Author: utnso
 */

#include <shared/socket.h>

int main2(void){

	int socket = socket_connect_to_server("127.0.0.1",  "8000" );
	t_header paquete;
	paquete.emisor = KERNEL;
	send(socket,&paquete,sizeof(t_header),0);
	paquete.tipo_mensaje = CONEXION;
	send(socket,&paquete,sizeof(t_header),0);
	paquete.tipo_mensaje = DESCONEXION;
	send(socket,&paquete,sizeof(t_header),0);

	while(1){};

	return 0;
}

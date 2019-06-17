#include "kernel.h"

int main() {

	inicializar_logs_y_configs();
	
	inicializar_kernel();

	//INICIA CLIENTE MEMORIA
	/*conectar_memoria();*/

	//INICIA CONSOLA
	pthread_t hilo_consola;
	pthread_create(&hilo_consola, NULL , (void*) consola, NULL);
	log_info(logger, "Kernel > Hilo creado de consola...");

	
	//INICIA HILOS DE EJECUCION
	crear_procesadores();

	pthread_join(hilo_consola, NULL);
	log_info(logger, "FIN hilo consola");

	liberar_kernel();

	return EXIT_SUCCESS;
}


void ejecutar_procesador(){

	while(1){

		log_info(logger, "Esperando pcb...");
		pthread_mutex_lock(&sem_ejecutar);

			t_PCB* pcb = obtener_pcb_ejecutar();
			log_info(logger, "Se obtiene para ejecutar pcb id: %d", pcb->id);
		pthread_mutex_unlock(&sem_ejecutar);

		if( pcb->tipo_request == SIMPLE ){
			log_info(logger, "Se recibe a ejecucion request simple: %s" , pcb->request_comando  );
			ejecutar_linea( pcb->request_comando );



			finalizar_pcb(pcb);
			//TODO:finalizar pcb. quitarlo de listos y pasarlo a fin
		}
		else
		{
			int k= 0;
			log_info(logger, "Se recibe a ejecucion request compuesta archivo: %s" , pcb->request_comando  );

			while( k < kernel_config.MULTIPROCESAMIENTO ){

				//TODO aca habria que obtener la linea a leer en el archivo segun el PC del pcb
				char* linea=NULL;
				log_info(logger, "la linea a ejecutar es: %s" , linea  );

				ejecutar_linea( linea );

				k++;
			}

			//TODO: verificar si es la ultima linea de archivo. si es asi quitalro de listos y enviarlos a fin sino devolverlo a listos
		}
	}
}


void ejecutar_linea( char *linea ){

	char* n_tabla = obtener_nombre_tabla( linea );

	t_tabla_consistencia *tabla = obtener_tabla( n_tabla );

	t_memoria_del_pool *memoria = obtener_memoria_criterio( tabla );

	ejecutar_linea_memoria( memoria , linea );
}


char* obtener_nombre_tabla( char* linea ){

	return NULL;
}

t_tabla_consistencia *obtener_tabla( char* n_tabla ){

	return NULL;
}

t_memoria_del_pool *obtener_memoria_criterio( t_tabla_consistencia* tabla ){

	return NULL;
}

void ejecutar_linea_memoria( t_memoria_del_pool* memoria , char* linea ){

}


t_PCB* obtener_pcb_ejecutar(){

	//si lista vacia se queda loopeando esperando que entre alguno
	while( list_is_empty( l_pcb_listos ) ){

	}
	log_info(logger, "tamanio de la lista de listos %d", list_size( l_pcb_listos ));

	t_PCB *pcb = list_remove( l_pcb_listos , 0 );
	list_add( l_pcb_ejecutando , pcb  );
	log_info(logger, "se agrega a ejecucion pcb id %d",  pcb->id );
	log_info(logger, "nuevo tamanio de la lista de listos %d", list_size( l_pcb_listos ));

	return pcb;
}

void finalizar_pcb(t_PCB* pcb){

	bool buscar_pcb( t_PCB* pcb_it ){

		if(  pcb_it->id == pcb->id  ) return true;
		return false;
	}

	log_info(logger, "tamanio de lista ejecutando: %d", list_size( l_pcb_ejecutando ));
	//quito de la lista pcb y lo agrego a finalizados
	list_remove_by_condition(l_pcb_ejecutando,(void*)buscar_pcb);
	log_info(logger, "nuevo tamanio de lista ejecutando: %d", list_size( l_pcb_ejecutando ));
	list_add( l_pcb_finalizados , pcb );
	log_info(logger, "tamanio lista finalizados: %d", list_size( l_pcb_finalizados ));

}

void inicializar_kernel(){

	id_pcbs = 0;
	pthread_mutex_init(&sem_ejecutar, NULL);

	//INIT lista criterios
	l_criterio_SHC = list_create();
	l_criterio_SC = list_create();
	l_criterio_EC = list_create();

	//INIT lista de estados
	l_pcb_nuevos = list_create();
	l_pcb_listos = list_create();
	l_pcb_ejecutando = list_create();
	l_pcb_finalizados = list_create();

	l_memorias = list_create();

	l_procesadores = list_create();

	l_tablas = list_create();

}


void conectar_memoria(){
	log_info(logger, "entro a socket");
	socket_memoria = socket_connect_to_server(kernel_config.IP_MEMORIA, kernel_config.PUERTO_MEMORIA);
	log_info(logger, "El socket devuelto es: %d", socket_memoria);
	if( socket_memoria == -1  ){

		log_error(logger, "¡Error no se pudo conectar con MEMORIA");
		//TODO: habria que verificar si aca se cierra todo para no tener leaks
		exit(EXIT_FAILURE);
	}
	log_info(logger, "Se creo el socket cliente con MEMORIA de numero: %d", socket_memoria);

	t_header buffer;
	buffer.emisor = KERNEL;
	buffer.tipo_mensaje = CONEXION;
	buffer.payload_size = 32;
	send(socket_memoria, &buffer, sizeof(buffer), 0);
	//TODO habria que realizar aca el handshake con memoria.

}

void crear_procesadores(){
	for(int i=0; i<kernel_config.MULTIPROCESAMIENTO; i++){
		pthread_t hilo_ejecucion;
		pthread_create(&hilo_ejecucion, NULL , (void*) ejecutar_procesador, NULL);
		log_info(logger, "Hilo de ejecucion creado id: %d" , hilo_ejecucion);
		list_add(l_procesadores, &hilo_ejecucion);
		pthread_detach(hilo_ejecucion);
	}
}


void ejecutar_describe(){

}


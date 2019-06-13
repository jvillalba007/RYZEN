#include "kernel.h"

int main() {

	inicializar_logs_y_configs();
	
	inicializar_kernel();

	//INICIA CLIENTE MEMORIA
	conectar_memoria();

	//INICIA CONSOLA
	pthread_t hilo_consola;
	pthread_create(&hilo_consola, NULL , (void*) consola, NULL);
	log_info(logger, "Kernel > Hilo creado de consola...");

	
	//INICIA HILOS DE EJECUCION
	crear_procesadores();


	//TODO: hagan limpieza en consola, fijense los comentarios que deje y prueben si funciona insertar una request de tipo select, crear el cpb ponerlo en lista de nuevos
	//y luego pasarlo a lista de ready verifiquen si hay leaks. Pasen el valgrind. pero pruebenlo, no sigan codeando de mas si no funciona todo ok.



	//TODO funcion para cerrar todo sin leaks. las listas, semaforos...etc

	pthread_join(hilo_consola, NULL);
	log_info(logger, "FIN hilo consola");

	liberar_kernel();

	//	Se liberan los char* pedidos anteriormente
	liberar_kernel_config(kernel_config);

	//	Log: clausura
	log_destroy(logger);
	
	return EXIT_SUCCESS;
}


void ejecutar(){

	while(1){

		pthread_mutex_lock(&sem_ejecutar);
			t_PCB* pcb = obtener_pcb_ejecutar();
			log_info(logger, "Se obtiene para ejecutar pcb id: %d", pcb->id);
		pthread_mutex_unlock(&sem_ejecutar);

		//TODO aca se haria la ejecucion del proceso

	}
	pthread_exit(0);
}


t_PCB* obtener_pcb_ejecutar(){

	//si lista vacia se queda loopeando esperando que entre alguno
	while( list_is_empty( l_pcb_listos ) ){

	}
	log_info(logger, "tamanio de la lista de listos %d", list_size( l_pcb_listos ));

	t_PCB *pcb = list_remove( l_pcb_listos , 0 ); //tomo primero por ser RR  TODO verificar si esto funciona
	list_add( l_pcb_ejecutando , pcb  );
	log_info(logger, "se agrega a ejecucion pcb id %d",  pcb->id );

	return pcb;
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

}


void conectar_memoria(){

	socket_memoria = socket_connect_to_server(kernel_config.IP_MEMORIA, kernel_config.PUERTO_MEMORIA);

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
		pthread_create(&hilo_ejecucion, NULL , (void*) ejecutar, NULL);
		log_info(logger, "Hilo de ejecucion creado");
		list_add(l_procesadores, &hilo_ejecucion);
		pthread_detach(hilo_ejecucion);
	}
}

void liberar_kernel(){

	//FIN lista criterios
		list_destroy(l_criterio_SHC);
		list_destroy(l_criterio_SC);
		list_destroy(l_criterio_EC);

	//FIN lista de estados
		list_destroy_and_destroy_elements(l_pcb_nuevos, free_Pcb);
		list_destroy_and_destroy_elements(l_pcb_listos, free_Pcb);
		list_destroy_and_destroy_elements(l_pcb_ejecutando, free_Pcb);
		list_destroy_and_destroy_elements(l_pcb_finalizados, free_Pcb);

		list_destroy(l_memorias);

		list_destroy(l_procesadores);
}

void free_Pcb(void* pcb_Void){
	t_PCB* pcb = (t_PCB*) pcb_Void;
	free(pcb->request_comando);
	free(pcb);
}

//	Lineamientos del Kernel
//	https://docs.google.com/document/d/1nus6JWLj3mjzUATejoatIK_qFewLztolEKRa0jXsa0I

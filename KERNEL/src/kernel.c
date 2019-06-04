#include "kernel.h"

int main() {
	//	Se inicializan los logs y las configs
	inicializar_logs_y_configs();
	
	//	Conexión con la memoria
	int socket = socket_connect_to_server(kernel_config.IP_MEMORIA, kernel_config.PUERTO_MEMORIA);
	t_header buffer;
	buffer.emisor = KERNEL;
	buffer.tipo_mensaje = CONEXION;
	buffer.payload_size = 32;
	send(socket, &buffer, sizeof(buffer), 0);
	
	//	Hilo para la consola
	pthread_attr_t atributos_consola;
	pthread_attr_init(&atributos_consola);
	pthread_t hilo_consola;
	pthread_create(&hilo_consola, &atributos_consola, (void*) consola, NULL);
	log_info(logger, "Kernel > Hilo creado para la consola...");

	
	//	Se espera a que el hilo termine...
	pthread_join(hilo_consola, NULL);

	puts("3...");
	sleep(1);
	puts("2...");
	sleep(1);
	puts("1...");
	sleep(1);

	//	Se liberan los char* pedidos anteriormente
	liberar_kernel_config(kernel_config);

	//	Log: clausura
	log_destroy(logger);
	
	return EXIT_SUCCESS;
}


//	Lineamientos del Kernel
//	https://docs.google.com/document/d/1nus6JWLj3mjzUATejoatIK_qFewLztolEKRa0jXsa0I
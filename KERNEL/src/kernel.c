#include "kernel.h"

int main() {
	//	Log: apertura
	abrir_log();

	//	Archivo de Configuración: creación y lectura del contenido
	crear_config();
	leer_configs();
	
	//	Log: registro inicial + registro de las configuraciones leídas
	loggear_inicio_logger();
	loggear_configs();	
	
	//	Conexión con la memoria
	int socket = socket_connect_to_server(kernel_config.IP_MEMORIA, kernel_config.PUERTO_MEMORIA);
	t_header buffer;
	buffer.emisor = KERNEL;
	buffer.tipo_mensaje = CONEXION;
	buffer.payload_size = 32;
	send(socket, &buffer, sizeof(buffer), 0);
	
	//	Hilo para la consola
	pthread_t hilo_consola;
	pthread_create(&hilo_consola, NULL, (void*) consola, NULL);
	log_info(logger, "KERNEL > Hilo creado para la consola...");


/*
	//	Hilo para la consola
	pthread_create(&tID, &attr, consola, NULL);
	
	//	Se espera a que el hilo termine...
	pthread_join(tID, NULL);
*/

	sleep(3);



	//	Se liberan los char* pedidos anteriormente
	liberar_kernel_config(kernel_config);

	//	Log: clausura
	log_destroy(logger);
	
	return EXIT_SUCCESS;
}



/* otra consola...

void consola(size_t TAMANIO_BUFFER) {
	//	Un bucle infinito porque la consola permanecerá abierta (casi) siempre.
	while (1) {
		char* buffer_linea_leida;
		size_t cantidad_caracteres;
		buffer_linea_leida = (char*) malloc(TAMANIO_BUFFER * 2 * sizeof(char));
		cantidad_caracteres = console(&buffer_linea_leida);
		log_info(logger, buffer_linea_leida);

		//	A continuación se empieza a parsear lo escrito por consola...
		//	Si se lee el comando EXIT, se saldrá del bucle.
		if (strcmp(buffer_linea_leida, "EXIT\n") == 0) {
			puts("Cerrando consola en 3...");
			sleep(1);
			puts("2...");
			sleep(1);
			puts("1...");
			sleep(1);
			break;
		}
		
		//	Si se lee el comando DESCRIBE (sin recibir parámetros)...
		if (strcmp(buffer_linea_leida, "DESCRIBE\n") == 0) {
			//	--> MEMORIA --> "obtener la metadata de todas las tablas que tenga el FS" --> LFS
			puts("Pusiste un DESCRIBE\n");
		}
		
		//	Si se lee el comando JOURNAL...
		if (strcmp(buffer_linea_leida, "JOURNAL\n") == 0) {
			//	"enviar JOURNAL a cada memoria asociada" 
			puts("Pusiste un JOURNAL\n");
		}
		
		//	Si se lee el comando METRICS...
		if (strcmp(buffer_linea_leida, "METRICS\n") == 0) {
			//	llamar a función mostrarMetricasActuales()
			puts("Pusiste un METRICS\n");
		}
		
		//	Acá viene la posta de la milanesa del parser...
		//	Ver funciones de las commons para laburar con strings: string_starts_with y string_split
		if (string_starts_with(buffer_linea_leida, "SELECT")) {
			puts("Pusiste un SELECT\n");
		}

		//	Acá se termina de parsear.

		free(buffer_linea_leida);
	}

	pthread_exit(0);
}
*/
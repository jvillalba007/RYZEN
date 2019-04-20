#include "kernel.h"

int main() {
	//	Log: apertura
	abrirLog();

	//	Archivo de Configuración: creación y lectura del contenido
	crearConfig();
	leerConfigs();
	
	//	Registros en el Log: inicial + configuraciones leídas
	loggearInicioLogger();
	loggearConfigs();	
	
/*
	//	Un hilo para abrir la consola
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_t tID;
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

/*void consola(size_t T) {
	// cambiar T por TAMANIO_BUFFER
	//	Un bucle infinito porque la consola permanecerá abierta (casi) siempre.
	while (1) {
		char* bufferLineaLeida;
		size_t cantCaracteres;
		bufferLineaLeida = (char*) malloc(TAMANIO_BUFFER * 2 * sizeof(char));
		cantCaracteres = console(&bufferLineaLeida);
		log_info(logger, bufferLineaLeida);

		//	A continuación se empieza a parsear lo escrito por consola...
		//	Si se lee el comando EXIT, se saldrá del bucle.
		if (strcmp(bufferLineaLeida, "EXIT\n") == 0) {
			puts("Cerrando consola en 3...");
			sleep(1);
			puts("2...");
			sleep(1);
			puts("1...");
			sleep(1);
			break;
		}
		
		//	Si se lee el comando DESCRIBE (sin recibir parámetros)...
		if (strcmp(bufferLineaLeida, "DESCRIBE\n") == 0) {
			//	--> MEMORIA --> "obtener la metadata de todas las tablas que tenga el FS" --> LFS
			puts("Pusiste un DESCRIBE\n");
		}
		
		//	Si se lee el comando JOURNAL...
		if (strcmp(bufferLineaLeida, "JOURNAL\n") == 0) {
			//	"enviar JOURNAL a cada memoria asociada" 
			puts("Pusiste un JOURNAL\n");
		}
		
		//	Si se lee el comando METRICS...
		if (strcmp(bufferLineaLeida, "METRICS\n") == 0) {
			//	llamar a función mostrarMetricasActuales()
			puts("Pusiste un METRICS\n");
		}
		
		//	Acá viene la posta de la milanesa del parser...
		//	Ver funciones de las commons para laburar con strings: string_starts_with y string_split
		if (string_starts_with(bufferLineaLeida, "SELECT")) {
			puts("Pusiste un SELECT\n");
		}

		//	Acá se termina de parsear.

		free(bufferLineaLeida);
	}

	pthread_exit(0);
}
*/
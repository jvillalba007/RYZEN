#include "kernel.h"

int main() {

	inicializar_logs_y_configs();
	inicializar_kernel();
/*
	//inicializo memoria y tabla de pruebas
	t_memoria_del_pool* memoria_sc = malloc( sizeof( t_memoria_del_pool ) );
	memoria_sc->activa=true;
	memoria_sc->numero_memoria=0;
	memoria_sc->criterio = strdup("EC");
	list_add(l_memorias , memoria_sc );

	t_tabla_consistencia* tabla = malloc( sizeof( t_tabla_consistencia ) );
	tabla->criterio_consistencia= strdup("SHC");
	tabla->nombre_tabla= strdup( "test" );
	list_add( l_tablas , tabla );

	t_memoria_del_pool* memoria_sc2 = malloc( sizeof( t_memoria_del_pool ) );
	memoria_sc2->activa=true;
	memoria_sc2->numero_memoria=1;
	memoria_sc2->criterio = strdup("EC");
	list_add(l_memorias , memoria_sc2 );
*/

	//INICIA CLIENTE MEMORIA
	/*conectar_memoria();*/

	//HILO REINICIO_ESTADISTICAS
	pthread_t hilo_reinicio_estadisticas;
	pthread_create(&hilo_reinicio_estadisticas, NULL, (void*) reinicio_estadisticas, NULL);
	log_info(logger,"iniciando hilo estadisticas %d", hilo_reinicio_estadisticas);

	//INICIA CONSOLA
	pthread_t hilo_consola;
	pthread_create(&hilo_consola, NULL , (void*) consola, NULL);
	log_info(logger, "Kernel > Hilo creado de consola...");

	
	//INICIA HILOS DE EJECUCION
	crear_procesadores();

	pthread_join(hilo_consola, NULL);
	log_info(logger, "FIN hilo consola");

	pthread_join(hilo_reinicio_estadisticas, NULL);
	log_info(logger, "fin hilo reinicio estadisticas");

	liberar_kernel();

	return EXIT_SUCCESS;
}


void ejecutar_procesador(){

	t_PCB* pcb = NULL;
	char* linea = NULL;

	while(!exit_global){

		log_info(logger, "Esperando pcb...");
		pthread_mutex_lock(&sem_ejecutar);

		pcb = obtener_pcb_ejecutar();

		pthread_mutex_unlock(&sem_ejecutar);

		if(pcb != NULL) {
		log_info(logger, "Se obtiene para ejecutar pcb id: %d", pcb->id);


			if( pcb->tipo_request == SIMPLE ){
				log_info(logger, "Se recibe a ejecucion request simple: %s" , pcb->request_comando  );
				ejecutar_linea( pcb->request_comando );

				finalizar_pcb(pcb);
			}
			else
			{
				int k= 0;

				char** split = string_split(pcb->request_comando, " ");
				log_info(logger, "Se recibe a ejecucion request compuesta archivo: %s" , pcb->request_comando  );
				FILE* archivo = fopen( split[1] , "r");
				apuntar_archivo(archivo, pcb->pc);
				split_liberar(split);

				while( (k < kernel_config.QUANTUM) && (!feof(archivo)) ){

					linea = obtener_linea(archivo);

					if(linea != NULL){

					log_info(logger, "la linea a ejecutar es: %s" , linea  );
					int res = ejecutar_linea( linea );
					k++;
					pcb->pc++;
					free(linea);

					if(res == 0){
						log_info(logger, "la linea se ejecuto correctamente");
					}else {
						finalizar_pcb(pcb);
						k = kernel_config.QUANTUM;
					}
					}
				}

				if(feof(archivo)){
					finalizar_pcb(pcb);
				} else{
					parar_por_quantum(pcb);
				}
				fclose(archivo);
			}
			free(pcb->request_comando);
			free(pcb);
		}

	}
	log_info(logger,"cerrando hilo");
	pthread_exit(0);
}


int ejecutar_linea( char *linea ){

	int res;
	t_tabla_consistencia *tabla=NULL;
	t_memoria_del_pool *memoria=NULL;

	char** parametros = string_split(linea, " ");

	char* n_tabla = obtener_nombre_tabla( parametros );

	if( n_tabla != NULL ) tabla = obtener_tabla( n_tabla );

	if( tabla != NULL ){
		log_info(logger, "Tabla encontrada: %s", tabla->nombre_tabla );
		memoria = obtener_memoria_criterio( tabla, linea);

		if( memoria != NULL ){

			log_info(logger, "Memoria a ejecutar: %d", memoria->numero_memoria );
			res = ejecutar_linea_memoria( memoria , linea );
		}
		else{
			//TODO: si memoria es null definir que hacer. Supongo que no va a realizar nada .
			log_info(logger, "Memoria para ejecutar no encontrada" );
			res=0;
		}
	}
	//Tabla es null si es un select drop o insert rompo
	else{
		log_info(logger, "No se encuentra la tabla" );
		if( string_equals_ignore_case( parametros[0], "SELECT") || string_equals_ignore_case( parametros[0], "INSERT") || string_equals_ignore_case( parametros[0], "DROP") ){
			log_info(logger, "Se cancela ejecucion de operacion: %s", parametros[0] );
			res= -1;
		}
		//es un describe general o de una tabla que no esta en sistema.
		else
		{
			log_info(logger, "es un DESCRIBE se ejecuta memoria random" );
			memoria = obtener_memoria_EC();
			if( memoria != NULL ){

				log_info(logger, "Memoria a ejecutar: %d", memoria->numero_memoria );
				res = ejecutar_linea_memoria( memoria , linea );
			}
			else{
				//TODO: si memoria es null definir que hacer. Supongo que no va a realizar nada .
				log_info(logger, "Memoria para ejecutar no encontrada" );
				res=0;
			}
		}
	}

	split_liberar(parametros);
	free(n_tabla);

	return res;
}


char* obtener_nombre_tabla( char** parametros){

	char* n_tabla=NULL;

	if( string_equals_ignore_case(parametros[0], "DESCRIBE") && split_cant_elem(parametros) < 2  ){
		log_info(logger, "DESCRIBE general sin nombre de tabla" );
		return n_tabla;
	}

	n_tabla= strdup(parametros[1]);
	log_info(logger, "tabla de request:%s", n_tabla );

	return n_tabla;
}

t_tabla_consistencia *obtener_tabla( char* n_tabla ){

	t_tabla_consistencia *tabla=NULL;

	bool buscar_tabla( t_tabla_consistencia* tabla_it ){

		if( string_equals_ignore_case( n_tabla , tabla_it->nombre_tabla )  ) return true;
		return false;
	}

	if(!list_is_empty(l_tablas))
	{
		tabla = list_find(l_tablas , (void*)buscar_tabla);
	}

	return tabla;
}

t_memoria_del_pool *obtener_memoria_criterio( t_tabla_consistencia* tabla, char* linea){

	t_memoria_del_pool *memoria=NULL;

	if( string_equals_ignore_case( tabla->criterio_consistencia ,"SC" ) ){

		memoria = obtener_memoria_SC();
	}

	else if(string_equals_ignore_case( tabla->criterio_consistencia ,"EC") ){

		memoria = obtener_memoria_EC();
	}

	else if( string_equals_ignore_case( tabla->criterio_consistencia ,"SHC") ){

		memoria = obtener_memoria_SHC(linea);
	}

	return memoria;
}

t_memoria_del_pool *obtener_memoria_SC( t_tabla_consistencia* tabla ){


	if( !list_is_empty( l_criterio_SC ) ) return list_get( l_criterio_SC , 0 );

	return NULL;
}

t_memoria_del_pool *obtener_memoria_EC(){

	t_memoria_del_pool* mem = NULL;
	if( !list_is_empty( l_criterio_EC)){
		int index = rand_num(list_size( l_criterio_EC));
		mem = list_get( l_criterio_EC, index);
	}
	return mem;
}

t_memoria_del_pool *obtener_memoria_SHC(char* linea){
	t_memoria_del_pool* mem = NULL;
	int index;
	char** split = string_split( linea, " ");
	log_info(logger, "%d", list_size(l_criterio_SHC));
	if( !list_is_empty( l_criterio_SHC )){
		if( string_equals_ignore_case(split[0], "INSERT") || string_equals_ignore_case(split[0], "SELECT") ){
			index = atoi(split[2]) % list_size( l_criterio_SHC);
			mem = list_get( l_criterio_SHC, index);
		} else{
			index = rand_num(list_size( l_criterio_SHC));
			mem = list_get( l_criterio_SHC, index);
		}
	}

	split_liberar(split);
	return mem;
}

int ejecutar_linea_memoria( t_memoria_del_pool* memoria , char* linea ){


	int socket;

	if(memoria->socket != -1){
		socket = memoria->socket;
	}
	else{
		socket = socket_connect_to_server(memoria->ip, memoria->puerto);
		log_info(logger, "El socket devuelto es: %d", socket);
		if( socket == -1  ){

			log_error(logger, "¡Error no se pudo conectar con MEMORIA");
			//TODO: ojo aca no hay que hacer un exit hay que hacer un return -1 , hay que desactivar la memoria y quitarla del pool del criterio donde estas
			exit(EXIT_FAILURE);

		}
		log_info(logger, "Se creo el socket cliente con MEMORIA de numero: %d", socket);

		memoria->socket = socket_memoria;
		memoria->activa = true;
	}

	char** split = string_split(linea, " ");

	if(es_string(split[0], "INSERT")){


		int tiempo_ejecucion = clock();
		int tamanio;
		char* buffer = convertir_insert(split);
		tamanio = sizeof(buffer);

		t_header *paquete = malloc(sizeof(t_header));
		paquete->emisor = KERNEL;
		paquete->tipo_mensaje = INSERT;
		paquete->payload_size = tamanio;
		send(socket, &paquete, sizeof(buffer), 0);
		free(paquete);

		send(socket, &buffer, tamanio, 0);
		operaciones_totales++;
		memoria->cantidad_carga++;
		free(buffer);
		memoria->cantidad_insert++;
		memoria->tiempo_insert = (clock() - tiempo_ejecucion)/memoria->cantidad_insert;


	}
	else if(es_string(split[0], "SELECT")){

		int tiempo_ejecucion = clock();

		int tamanio;
		char* buffer = convertir_select(split);
		tamanio = sizeof(buffer);

		t_header *paquete = malloc(sizeof(t_header));
		paquete->emisor = KERNEL;
		paquete->tipo_mensaje = SELECT;
		paquete->payload_size = tamanio;

		send(socket, &paquete, sizeof(buffer), 0);
		free(paquete);

		send(socket, &buffer, tamanio, 0);
		free(buffer);
		operaciones_totales++;
		memoria->cantidad_carga++;
		memoria->cantidad_select++;
		memoria->tiempo_select = (clock() - tiempo_ejecucion)/memoria->cantidad_select;

	}
	else if(es_string(split[0], "CREATE")){

		int tamanio;
		char* buffer = convertir_create(split);
		tamanio = sizeof(buffer);

		t_header *paquete = malloc(sizeof(t_header));
		paquete->emisor = KERNEL;
		paquete->tipo_mensaje = CREATE;
		paquete->payload_size = tamanio;
		send(socket, &paquete, sizeof(buffer), 0);
		free(paquete);

		send(socket, &buffer, tamanio, 0);
		operaciones_totales++;
		free(buffer);

	}
	else if(es_string(split[0], "DROP")){

		char* tabla = split[1];
		int size= sizeof( tabla );

		t_header paquete;
		paquete.emisor = KERNEL;
		paquete.tipo_mensaje = DROP;
		paquete.payload_size = size;

		send(socket, &paquete, sizeof(paquete), 0);

		//TODO: falta el segundo send
		//TODO: en el primer send al menos habria que verificar si se pudo hacer ya que si algo falla hay que abortar el proceso...esto es importante. Esto es en todos los envios

		operaciones_totales++;
	}
	else if(es_string(split[0], "DESCRIBE")){

		operaciones_totales++;
	}
	else{
		log_error(logger,"comando no reconocido");
	}

	split_liberar(split);
	return 0;
}


t_PCB* obtener_pcb_ejecutar(){

	//si lista vacia se queda loopeando esperando que entre alguno
	while( (list_is_empty( l_pcb_listos )) && (!exit_global) ){

	}

	if(exit_global) return NULL;

	t_PCB *pcb = NULL;
	log_info(logger, "tamanio de la lista de listos %d", list_size( l_pcb_listos ));
	pcb = list_remove( l_pcb_listos , 0 );
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
	operaciones_totales=0;
	exit_global = 0;

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
	t_memoria_del_pool* memoria_original = malloc( sizeof( t_memoria_del_pool ) );
	memoria_original->ip = kernel_config.IP_MEMORIA;
	memoria_original->puerto = kernel_config.PUERTO_MEMORIA;
	memoria_original->activa=true;
	memoria_original->numero_memoria=0;
	memoria_original->socket = socket_memoria;
	memoria_original->cantidad_carga = 0;
	list_add(l_memorias , memoria_original );

	t_header buffer;
	buffer.emisor = KERNEL;
	buffer.tipo_mensaje = CONEXION;
	buffer.payload_size = 32;
	send(socket_memoria, &buffer, sizeof(buffer), 0);
	//TODO habria que realizar aca el handshake con memoria.

}

void crear_procesadores(){

	pthread_t hilo_ejecucion;

	for(int i=0; i<kernel_config.MULTIPROCESAMIENTO; i++){

		pthread_create(&hilo_ejecucion, NULL , (void*) ejecutar_procesador, NULL);
		log_info(logger, "Hilo de ejecucion creado id: %d" , hilo_ejecucion);

		pthread_t* valor = malloc(sizeof(pthread_t));
		*valor = hilo_ejecucion;

		list_add(l_procesadores, valor);
		pthread_detach(hilo_ejecucion);
	}
}


void ejecutar_describe(){

}

char* obtener_linea(FILE* archivo){

	char leido;
	fread(&leido, sizeof(char),1,archivo);
	char* linea = string_from_format("%c", leido);
	while (!feof(archivo) && fread(&leido, sizeof(char),1,archivo) && leido != '\n'){
		string_append_with_format(&linea, "%c", leido);
	}

	return linea;
}

void apuntar_archivo(FILE* archivo, int pc){
	char leido;
	for(int i=0; i<pc; i++){
	while (!feof(archivo) && fread(&leido, sizeof(char),1,archivo) && leido != '\n'){}
	}

}

int rand_num(int max){
	int numero;
	numero = rand() % max;

	return numero;
}

void parar_por_quantum(t_PCB* pcb){

bool buscar_pcb( t_PCB* pcb_it ){

		if(  pcb_it->id == pcb->id  ) return true;
		return false;
	}
	list_remove_by_condition(l_pcb_ejecutando,(void*)buscar_pcb);
	log_info(logger, "nuevo tamanio de lista ejecutando: %d", list_size( l_pcb_ejecutando ));
	list_add( l_pcb_listos , pcb );
	log_info(logger, "tamanio lista de listos: %d", list_size( l_pcb_listos ));
}

char *convertir_insert(char** split){

	int tamanio;
	linea_insert insert;
	insert.tabla = split[1];
	insert.key = atoi(split[2]);
	insert.value = split[3];
	char *buffer = serializar_insert(insert,&tamanio);

	free(insert.tabla);
	free(insert.value);
	return buffer;
}

char *convertir_select(char** split){
	int tamanio;
	linea_select select;
	select.tabla = split[1];
	select.key = atoi(split[2]);
	char* buffer = serializar_select(select, &tamanio);

	free(select.tabla);
	return buffer;

}

char *convertir_create(char** split){
	int tamanio;
	linea_create create;
	create.tabla = split[1];
	create.tipo_consistencia = split[2];
	create.nro_particiones = atoi(split[3]);
	create.tiempo_compactacion = *(u_int32_t*)split[4];
	char* buffer = serializar_create(create, &tamanio);

	free(create.tabla);
	free(create.tipo_consistencia);
	return buffer;

}

void reinicio_estadisticas(){

	struct timespec time;
	time.tv_sec = 30;
	time.tv_nsec = 0;
	while(!exit_global){

		nanosleep(&time, NULL);
		log_info(logger, "iniciando reincio estadisticas");

		void reiniciar_memorias(t_memoria_del_pool* memoria)
		{
			log_info(logger,"reiniciando memoria: %d",memoria->numero_memoria);
			memoria->cantidad_insert = 0;
			memoria->cantidad_select = 0;
			memoria->tiempo_insert = 0;
			memoria->tiempo_select = 0;
		}

		list_iterate(l_memorias,(void*)reiniciar_memorias);
	}

	pthread_exit(0);
}



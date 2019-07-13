#include "kernel.h"


void handler(int id) {

}

void assignHandler() {
	struct sigaction sa = {0};
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
}


int main() {

	inicializar_logs_y_configs();
	inicializar_kernel();


	//INICIA CLIENTE MEMORIA
	//conectar_memoria();

	//HILO REINICIO_ESTADISTICAS
	pthread_create(&tid_estadisticas, NULL, (void*) reinicio_estadisticas, NULL);
	log_info(logger,"iniciando hilo estadisticas %d", tid_estadisticas);

	//HILO GOSSIPING
	pthread_create(&tid_gossiping, NULL, (void*) hilo_gossiping, NULL);
	log_info(logger,"iniciando hilo gossiping %d", tid_gossiping);

	//INICIA CONSOLA
	pthread_t hilo_consola;
	pthread_create(&hilo_consola, NULL , (void*) consola, NULL);
	log_info(logger, "Kernel > Hilo creado de consola...");

	//INICIA HILOS DE EJECUCION
	crear_procesadores();

	pthread_join(hilo_consola, NULL);
	pthread_join(tid_estadisticas, NULL);
	pthread_join(tid_gossiping, NULL);
	log_info(logger, "FIN hilo consola");
	log_info(logger, "FIN hilo reinicio estadisticas");
	log_info(logger, "FIN hilo gossiping");

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

	if (es_string(parametros[0],"CREATE")){ //AGREGO ESTO PORQUE SI ES UN CREATE NO VA A EXISTIR LA TABLA!

		memoria = obtener_memoria_criterio_create( parametros[2], linea);
		res = ejecutar_linea_memoria( memoria , linea );

	}
	else{
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
				log_info(logger, "es un DESCRIBE se ejecuta memoria random" );//TODO:decidir en que lista
				memoria = obtener_memoria_random( l_criterio_EC );
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

		free(n_tabla);
	}

	split_liberar(parametros);


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

t_memoria_del_pool *obtener_memoria_SC(){


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
			//TODO:falta confirmacion de si se saca o no del criterio
			log_error(logger, "¡Error no se pudo conectar con MEMORIA");
			desconectar_memoria(memoria);
			memoria->activa = false;
			return -1;

		}
		log_info(logger, "Se creo el socket cliente con MEMORIA de numero: %d", socket);

		memoria->socket = socket;
		memoria->activa = true;
	}

	char** split = string_split(linea, " ");

	if(es_string(split[0], "INSERT")){


		int tiempo_ejecucion = clock();

		linea_insert insert;
		insert.tabla = split[1];
		insert.key = atoi(split[2]);
		insert.value = split[3];

		enviar_insert(insert, &socket);
		free(insert.tabla);
		free(insert.value);

		operaciones_totales++;
		memoria->cantidad_carga++;
		memoria->cantidad_insert++;
		memoria->tiempo_insert = (clock() - tiempo_ejecucion)/memoria->cantidad_insert;


	}
	else if(es_string(split[0], "SELECT")){

		int tiempo_ejecucion = clock();

		linea_select select;
		select.tabla = split[1];
		select.key = atoi(split[2]);

		enviar_select(select, &socket);

		free(select.tabla);

		int tamanio;
		recv(socket, &tamanio, sizeof(int), MSG_WAITALL);
		char* buffer = malloc(tamanio+1);
		recv(socket, buffer, tamanio, MSG_WAITALL);

		linea_response_select *response_select = malloc(sizeof(linea_response_select));
		deserializar_response_select(buffer, response_select);
		log_info(logger, "tabla %s con value %s", split[1], response_select->value);

		free(buffer);
		free(response_select->value);

		operaciones_totales++;
		memoria->cantidad_carga++;
		memoria->cantidad_select++;
		memoria->tiempo_select = (clock() - tiempo_ejecucion)/memoria->cantidad_select;

	}
	else if(es_string(split[0], "CREATE")){



		linea_create create;
		create.tabla = split[1];
		create.tipo_consistencia = split[2];
		create.nro_particiones = atoi(split[3]);
		create.tiempo_compactacion = *(u_int32_t*)split[4];

		enviar_create(create, &socket);

		int respuesta;
		recv(socket, &respuesta, sizeof(int), MSG_WAITALL);
		if (respuesta>0){
			t_tabla_consistencia *tabla = malloc(sizeof(t_tabla_consistencia));
			tabla->criterio_consistencia = split[2];
			tabla->nombre_tabla = split[1];
			list_add(l_tablas, tabla);
			log_info(logger,"tabla %s creada", split[1]);
		}
		else{
			log_error(logger,"no se pudo crear la tabla %s", split[1]);
		}
		free(create.tabla);
		free(create.tipo_consistencia);

		operaciones_totales++;


	}
	else if(es_string(split[0], "DROP")){

		char* tabla = split[1];

		enviar_drop(&socket, tabla);

		int respuesta;
		recv(socket, &respuesta, sizeof(int), MSG_WAITALL);
		if(respuesta>0){
			log_info(logger,"se hizo drop de la tabla %s",split[1]);
		}else{
			log_error(logger,"no se realizo correctamente el drop de la tabla %s",split[1]);
		}

		//TODO: en el primer send al menos habria que verificar si se pudo hacer ya que si algo falla hay que abortar el proceso...esto es importante. Esto es en todos los envios
		operaciones_totales++;
	}
	else if(es_string(split[0], "DESCRIBE")){

		if(split[1] == NULL){

			log_info(logger,"Ejecuto DESCRIBE general");
			enviar_describe_general(&socket);

			int tamanio;
			recv(socket, &tamanio, sizeof(int), MSG_WAITALL);
			char *buffer = malloc(tamanio+1);
			recv(socket, buffer, tamanio, MSG_WAITALL);

			t_list *lista_tablas = deserializar_describe(buffer);

			list_iterate( lista_tablas , (void*)agregar_tabla_describe );
			log_info(logger,"Se termino de ejecutar DESCRIBE general");

			list_destroy_and_destroy_elements( lista_tablas , (void*)free_tabla_describe);

		}else{

			enviar_describe_especial(&socket, split[1]);

			int tamanio;
			recv(socket, &tamanio, sizeof(int), MSG_WAITALL);
			char *buffer = malloc(tamanio+1);
			recv(socket, buffer, tamanio, MSG_WAITALL);

			t_list *lista_tablas = deserializar_describe(buffer);
			list_iterate( lista_tablas , (void*)agregar_tabla_describe );
			log_info(logger,"Se termino de ejecutar DESCRIBE");
			list_destroy_and_destroy_elements( lista_tablas , (void*)free_tabla_describe);
		}
		operaciones_totales++;
	}
	else{
		log_error(logger,"comando no reconocido");
	}

	split_liberar(split);
	return 0;
}



void agregar_tabla_describe( linea_create* tabla_describe ){

	bool tabla_encontrada( t_tabla_consistencia *tabla ){

		if( string_equals_ignore_case(tabla->nombre_tabla , tabla_describe->tabla ) ) return true;
		return false;
	}

	//si no la encuentra la agrego a la lista
	if( list_find( l_tablas , (void*)tabla_encontrada ) == NULL ){

		t_tabla_consistencia *tabla_nueva = malloc( sizeof(t_tabla_consistencia ) );
		tabla_nueva->nombre_tabla = strdup( tabla_describe->tabla );
		tabla_nueva->criterio_consistencia = strdup( tabla_describe->tipo_consistencia );
		tabla_nueva->nro_particiones = tabla_describe->nro_particiones ;
		tabla_nueva->tiempo_compactacion = tabla_describe->tiempo_compactacion;

		list_add(l_tablas , tabla_nueva );
		log_info(logger, "se agrega la tabla :s",  tabla_nueva->nombre_tabla );
	}
	else{
		log_info(logger, "ya se encuentra en el sistema la tabla: %s",  tabla_describe->tabla );

	}

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
	socket_memoria = socket_connect_to_server("127.0.0.1", "8005");
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

	//hago gossiping con la memoria principal. TODO: verificar si esto debe estar en un loop hasta que logre conectarse realmente
	gossiping( memoria_original );
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

void enviar_insert(linea_insert linea, void* sock){

	int socket = *(int*)sock;
	int tamanio;
	char* buffer = serializar_insert( linea, &tamanio);

	t_header *paquete = malloc(sizeof(t_header));
	paquete->emisor = KERNEL;
	paquete->tipo_mensaje = INSERT;
	paquete->payload_size = tamanio;
	send(socket, &paquete, sizeof(t_header), 0);
	free(paquete);

	send(socket, &buffer, tamanio, 0);

	free(buffer);
}

void enviar_select(linea_select linea, void* sock){

	int socket = *(int*)sock;
	int tamanio;

	char* buffer = serializar_select(linea, &tamanio);

	t_header *paquete = malloc(sizeof(t_header));
	paquete->emisor = KERNEL;
	paquete->tipo_mensaje = SELECT;
	paquete->payload_size = tamanio;

	send(socket, &paquete, sizeof(t_header), 0);
	free(paquete);

	send(socket, &buffer, tamanio, 0);
	free(buffer);
}

void enviar_create(linea_create linea, void* sock){

	int socket = *(int*)sock;
	int tamanio;

	char* buffer = serializar_create(linea, &tamanio);

	t_header *paquete = malloc(sizeof(t_header));
	paquete->emisor = KERNEL;
	paquete->tipo_mensaje = CREATE;
	paquete->payload_size = tamanio;

	send(socket, &paquete, sizeof(t_header), 0);
	free(paquete);

	send(socket, &buffer, tamanio, 0);

	free(buffer);

}

void enviar_describe_general(void* sock){

	int socket = *(int*)sock;
	t_header *paquete = malloc(sizeof(t_header));
	paquete->emisor = KERNEL;
	paquete->tipo_mensaje = DESCRIBE;
	paquete->payload_size = 0;

	send(socket, &paquete, sizeof(t_header), 0);
	free(paquete);

}

void enviar_describe_especial(void* sock, char* tabla){

	int socket = *(int*)sock;
	int tamanio;
	char* buffer = serializar_string(tabla, &tamanio);

	t_header *paquete = malloc(sizeof(t_header));
	paquete->emisor = KERNEL;
	paquete->tipo_mensaje = DESCRIBE;
	paquete->payload_size = tamanio;

	send(socket, &paquete, sizeof(t_header), 0);
	free(paquete);

	send(socket, &buffer, tamanio, 0);
	free(buffer);
}

void reinicio_estadisticas(){

	struct timespec ts;
	ts.tv_sec = kernel_config.METADATA_REFRESH / 1000;
	ts.tv_nsec = (kernel_config.METADATA_REFRESH % 1000) * 1000000;

	assignHandler();

	while(!exit_global){

		nanosleep(&ts, NULL);
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
void enviar_drop(void* sock,char* tabla){

	int socket = *(int*)sock;
	int tamanio;
	char* buffer = serializar_string(tabla, &tamanio);

	t_header *paquete = malloc(sizeof(t_header));
	paquete->emisor = KERNEL;
	paquete->tipo_mensaje = DROP;
	paquete->payload_size = tamanio;

	send(socket, &paquete, sizeof(t_header), 0);
	free(paquete);

	send(socket, &buffer, tamanio, 0);
	free(buffer);
}

t_memoria_del_pool *obtener_memoria_criterio_create(char* criterio, char* linea){

	t_memoria_del_pool *memoria=NULL;

	if( string_equals_ignore_case( criterio ,"SC" ) ){

		memoria = obtener_memoria_SC();
	}

	else if(string_equals_ignore_case( criterio ,"EC") ){

		memoria = obtener_memoria_EC();
	}

	else if( string_equals_ignore_case( criterio ,"SHC") ){

		memoria = obtener_memoria_SHC(linea);
	}

	return memoria;
}

void recibir_agregar_memoria(void* sock){
	int socket_memoria = *(int*)sock;

	int tamanio_buffer;
	recv(socket_memoria, &tamanio_buffer, sizeof(int), MSG_WAITALL);

	char *buffer = malloc(tamanio_buffer + 1);
	recv(socket_memoria, buffer, tamanio_buffer, MSG_WAITALL);

	pmemoria memoria_recibir;
	deserializar_memoria(buffer, &memoria_recibir);

	t_memoria_del_pool* memoria = malloc( sizeof( t_memoria_del_pool ) );
	memoria->ip = memoria_recibir.ip;
	memoria->puerto = memoria_recibir.puerto;
	memoria->numero_memoria=0;
	memoria->activa = memoria_recibir.activa;
	if(memoria_recibir.activa){
		int socket = socket_connect_to_server(memoria->ip, memoria->puerto);
		log_info(logger, "El socket devuelto es: %d", socket);
		if( socket == -1  ){

			log_error(logger, "¡Error no se pudo conectar con MEMORIA");

			memoria->socket = socket;
		}else{
		log_info(logger, "Se creo el socket cliente con MEMORIA de numero: %d", socket_memoria);
		memoria->socket = socket;
		}
	}else{

		memoria->socket = -1;
	}

	memoria->cantidad_carga = 0;
	memoria->cantidad_insert = 0;
	memoria->cantidad_select = 0;
	list_add(l_memorias , memoria );
	free(buffer);

}


void hilo_gossiping(){

	struct timespec ts;
	ts.tv_sec = kernel_config.GOSSIPING_REFRESH / 1000;
	ts.tv_nsec = (kernel_config.GOSSIPING_REFRESH % 1000) * 1000000;

	assignHandler();

	while(!exit_global){

		nanosleep(&ts, NULL);
		log_info(logger, "iniciando gossiping");

		t_memoria_del_pool *memoria = obtener_memoria_random( l_memorias );
		if( memoria== NULL ){
			log_info(logger, "No se encuentra memoria disponible para realizar gossiping");
			return;
		}

		log_info(logger, "La memoria random activa elegida es:%d",memoria->numero_memoria);
		int res = gossiping(memoria);
		if( res ==-1 ){

			log_info(logger, "Falla gossiping con memoria:%d",memoria->numero_memoria);
		}
		else{
			log_info(logger, "Se realiza gossiping exitosamente con memoria:%d",memoria->numero_memoria);
		}

	}

	pthread_exit(0);
}


int gossiping( t_memoria_del_pool *memoria ){

	log_info(logger, "memoria recibida para gossiping: %d" , memoria->numero_memoria);
	if( memoria->socket != -1 ){

		int socketmemoria = socket_connect_to_server(memoria->ip,  memoria->puerto );
		log_info(logger, "%d" ,socketmemoria);

		if( socketmemoria == -1 ){
			memoria->activa=-1;//verificar si debo sacarlas de los criterios o no...diria que no.
			log_info(logger, "no se pudo conectar con memoria. se rechaza gossiping");
			return -1;
		}
		log_info(logger, "Se establece conexion con memoria: %d: socket: %d" , memoria->numero_memoria , memoria->socket);
		memoria->socket=socketmemoria;
	}

	//genero intercambio
	t_header buffer;
	buffer.emisor=KERNEL;
	buffer.tipo_mensaje =  GOSSIPING;
	send(memoria->socket, &buffer, sizeof( buffer ) , 0);


	t_header header_tabla;
	recv(memoria->socket , &header_tabla, sizeof(t_header), MSG_WAITALL);

	char *buffer_tabla = malloc( header_tabla.payload_size);
	recv(memoria->socket , buffer_tabla, header_tabla.payload_size , MSG_WAITALL);

	t_list *memorias_seed = deserializar_memorias(buffer_tabla);

	//recorro lista de gossiping y agrego las nuevas
	list_iterate( memorias_seed , (void*)agregar_memoria_gossip );

	free( buffer_tabla );
	liberar_memorias_gossiping(memorias_seed);

	return 0;
}


void agregar_memoria_gossip( pmemoria *memoria ){

	bool memoria_encontrada( t_memoria_del_pool *memoria_pool ){

		if( memoria_pool->numero_memoria == memoria->numero_memoria ) return true;
		return false;
	}

	//si no la encuentra la agrego a la lista
	if( list_find( l_memorias , (void*)memoria_encontrada ) == NULL ){

		t_memoria_del_pool* memoria_nueva = malloc( sizeof( t_memoria_del_pool ) );
		memoria_nueva->activa=0; //desactivada ya que no esta asociada a ningun criterio
		memoria_nueva->numero_memoria=memoria->numero_memoria;
		memoria_nueva->ip = strdup(memoria->ip);
		memoria_nueva->puerto = strdup(memoria->puerto);
		memoria_nueva->cantidad_carga=0;
		memoria_nueva->cantidad_insert=0;
		memoria_nueva->cantidad_select=0;
		memoria_nueva->socket=-1;
		list_add(l_memorias , memoria_nueva );
		log_info(logger, "se agrega al pool la memoria: %d",  memoria_nueva->numero_memoria );
	}
	else{
		log_info(logger, "ya se encuentra en el pool la memoria: %d",  memoria->numero_memoria );
	}


}


void desconectar_memoria(t_memoria_del_pool* memoria){

	bool memoria_encontrada( t_memoria_del_pool *memoria_pool ){

		if( memoria_pool->numero_memoria == memoria->numero_memoria ) return true;
		return false;
	}

	if(es_string(memoria->criterio, "SHC")){

		list_remove_by_condition(l_criterio_SHC, (void*)memoria_encontrada);

	}else{
		if(es_string(memoria->criterio, "SC")){

			list_remove_by_condition(l_criterio_SC, (void*)memoria_encontrada);

		}else{

			list_remove_by_condition(l_criterio_EC, (void*)memoria_encontrada);

		}
	}
}


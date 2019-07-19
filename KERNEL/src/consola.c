#include "consola.h"
#include "kernel.h"

#define ever ;;
#define OR ||
#define AND &&

void consola() {
    
    while(!exit_global) {
        char* linea = console();

        if(es_string(linea, "EXIT")) {

        	exit_global=1;
        	pthread_kill(tid_estadisticas, SIGUSR1);
        	pthread_kill(tid_gossiping, SIGUSR1);
        	pthread_kill(tid_describe, SIGUSR1);
        	pthread_kill(tid_inotify, SIGUSR1);
            free(linea);
            break;
        }
        else if ( strcmp(linea, "") != 0){
        	procesar_comando(linea);
        }

        free(linea);
    }
    pthread_exit(0);
    //exit(EXIT_SUCCESS);
}

void procesar_comando (char* linea) {
    char** parametros = string_split(linea, " ");
    int cant_parametros = split_cant_elem(parametros);
    t_tipo_request tipo;
    
    //  Se verifica si es un comando correcto, de los conocidos
    if (es_comando_conocido(parametros)) {
        
        //  Se verifica si la cantidad de parámetros es correcta
        if (es_correcta_cantidad_parametros(parametros[0], cant_parametros)) {

            //  Se verifica si el comando es "planificable"
            if (es_comando_planificable(parametros[0])) {

                //  Se verifica si el comando es un RUN (request compuesta) u otro (request simple)
                if (es_string(parametros[0], "RUN")) {
                    tipo = COMPUESTA;

                    log_info(logger, "Es un RUN.");
                }
                else {
                    tipo = SIMPLE;
                    log_info(logger, "Es un CREATE, un INSERT, un SELECT, un DROP o un DESCRIBE.");
                }
                
                //  Se crea el PCB, se lo carga en la lista de NEW y se lo mueve a la lista de READY
                crear_pcb(linea, tipo);

            }
            else {

                if (es_string(parametros[0],"JOURNAL")) {

                	pthread_mutex_lock(&sem_memorias);
                		log_info(logger, "Ejecuto journal");
                		enviar_journal_lista_memorias(l_memorias);
                		log_info(logger, "Journal finalizado");
					pthread_mutex_unlock(&sem_memorias);
                }
				if (es_string(parametros[0], "ADD")) {

					pthread_mutex_lock(&sem_memorias);

                    //  ADD MEMORY numero_de_memoria TO criterio_de_consistencia
                    t_memoria_del_pool* m = obtener_memoria(atoi(parametros[2]));
                    if (m != NULL) {
                        
                        if (es_string(parametros[4], "SC") OR es_string(parametros[4], "SHC") OR es_string(parametros[4], "EC")) {
                            
                            if (es_string(parametros[4], "SC")) {
                                
                                if (list_is_empty(l_criterio_SC)) {
                                	log_info(logger, "Se agrega a criterio SC memoria: %d", m->numero_memoria);
                                    list_add(l_criterio_SC, m);
                                    m->activa= true;
                                }
                                else {
                                    log_info(logger, "Error en el comando ADD: Ya existe una memoria asignada al criterio SC.");
                                }
                                
                            }
                            else if (es_string(parametros[4], "SHC")) {

                            	list_add(l_criterio_SHC, m);
                            	m->activa=1;
                            	log_info(logger, "Se agrega a criterio SHC memoria: %d", m->numero_memoria);

                            	log_info(logger, "Inicio journal en criterio SHC");
								enviar_journal_lista_memorias(l_criterio_SHC);
								log_info(logger, "FIN journla en criteiro shc");
                            }
                            else {
                                log_info(logger, "Se agrega a criterio EC memoria: %d", m->numero_memoria);
                                list_add(l_criterio_EC, m);
                                m->activa= 1;
                            }
                        }
                        else {
                            //  Criterio de consistencia incorrecto
                            puts("Error en el comando ADD: criterio de consistencia desconocido.");
                        }
                    }
                    else {
                        //  El número de memoria NO está en la lista de memorias
                        log_info(logger, "Error en el comando ADD: número de memoria desconocido.");
                    }

                    pthread_mutex_unlock(&sem_memorias);

				}
				if (es_string(parametros[0], "METRICS")) {

					log_info(logger, "Ejecuto metrics");
					if(!list_is_empty(l_memorias)){
						list_iterate(l_memorias, (void*)ejecutar_metricas);
					}else{
						printf("No conozco ninguna memoria\n");
					}

				}
            }
        }
        else {
            notificar_error_comando_cantidad_parametros();
        }
    }
    else {
        notificar_error_comando_incorrecto();
    }

    split_liberar(parametros);
}

void crear_pcb (char* string_codigo, t_tipo_request tipo) {
    t_PCB* pcb = malloc(sizeof(t_PCB));
    pcb->id = id_pcbs;
    pcb->request_comando = strdup(string_codigo);
    pcb->pc = 0;
    pcb->tipo_request = tipo;  
    
    //  Se crea el PCB
    pthread_mutex_lock(&sem_pcb);
    	list_add(l_pcb_nuevos, pcb);
    	list_add(l_pcb_listos, list_remove(l_pcb_nuevos, 0));
    	log_info(logger, "Se crea el PCB de la request: %s con id: %d ", pcb->request_comando, pcb->id);
    	log_info(logger, "Cantida de pcb listos: %d ", list_size(l_pcb_listos));
	pthread_mutex_unlock(&sem_pcb);
    id_pcbs++;
}


t_memoria_del_pool* obtener_memoria(int numero_de_memoria) {
    t_memoria_del_pool* mem = NULL;

	bool buscar_memoria (t_memoria_del_pool *s) {
        if (s->numero_memoria == numero_de_memoria) {
            return true;
        } 
        else {
            return false;
        }
    }
    
    if (!list_is_empty(l_memorias)) {
        mem = list_find(l_memorias, (void*) buscar_memoria);
    }

    return mem;
}

void enviar_journal_lista_memorias (t_list* memorias) {
    
    void enviar_journal_memoria(t_memoria_del_pool* m) {

    	int res=0;
    	//intento conectarme
    	if( m->socket == -1 ){

    		int socketmemoria = socket_connect_to_server(m->ip,  m->puerto );
			if( socketmemoria == -1  ){

				m->socket=-1;
				log_error(logger, "¡Error no se pudo conectar con MEMORIA:%d" , m->numero_memoria);
				desactivar_memoria(m);
				log_info(logger, "Desactivo la memoria: %d . La quito de los criterios donde esta asociada" ,m->numero_memoria );
				return;
			}
			log_info(logger, "conexion existosa con memoria: %d de socket:", m->numero_memoria , m->socket );
			m->socket = socketmemoria;
    	}

		t_header header;
		header.emisor=KERNEL;
		header.tipo_mensaje = JOURNAL;
		header.payload_size = 0;
		res = send(m->socket, &header, sizeof( header ) , MSG_NOSIGNAL);
		if( res == -1 ){
			pthread_mutex_lock(&sem_memorias);
				desactivar_memoria(m);
			pthread_mutex_unlock(&sem_memorias);
			log_info(logger, "Fallo send de journal a la memoria %d", m->numero_memoria);
		}
		else{
			log_info(logger, "Se envió el journal a la memoria: %d", m->numero_memoria);
		}

   }


    t_list* l_memorias_activas = get_memorias_activas( memorias );

	if (l_memorias_activas == NULL) {

		log_info(logger, "No hay memorias activas para hacerles el journal.");
		list_destroy(l_memorias_activas);
		return;
	}

	log_info(logger, "Memorias activas: %d.", list_size(l_memorias_activas));
	list_iterate(l_memorias_activas, (void*) enviar_journal_memoria);
	list_destroy(l_memorias_activas);
}

bool es_comando_conocido (char** parametros) {
    if (es_string(parametros[0], "CREATE") OR
    es_string(parametros[0], "INSERT") OR
    es_string(parametros[0], "SELECT") OR
    es_string(parametros[0], "DROP") OR
    es_string(parametros[0], "RUN") OR
    es_string(parametros[0], "DESCRIBE")OR
    es_string(parametros[0], "JOURNAL") OR
    (   es_string(parametros[0], "ADD") AND
        es_string(parametros[1], "MEMORY") AND
        es_string(parametros[3], "TO")) OR
    es_string(parametros[0], "METRICS") OR
    es_string(parametros[0], "CLEAR")) {
        return true;
    }
    else {
        return false;
    }
}

bool es_correcta_cantidad_parametros (char* comando, int cantidad) {
    if (es_string(comando, "CREATE") AND (cantidad == 5)) {
        return true;
    }
    else if (es_string(comando, "INSERT") AND (cantidad == 4)) {
        return true;
    }
    else if (es_string(comando, "SELECT") AND (cantidad == 3)) {
        return true;
    }
    else if (es_string(comando, "DROP") AND (cantidad == 2)) {
        return true;
    }
    else if (es_string(comando, "RUN") AND (cantidad == 2)) {
        return true;
    }
    else if (es_string(comando, "DESCRIBE") AND ((cantidad == 1) OR (cantidad = 2))) {
        return true;
    }
    else if (es_string(comando, "JOURNAL") AND (cantidad == 1)) {
        return true;
    }
    else if (es_string(comando, "ADD") AND (cantidad == 5)) {
        return true;
    }
    else if (es_string(comando, "METRICS") AND (cantidad == 1)) {
        return true;
    }
    else if (es_string(comando, "CLEAR") AND (cantidad == 1)) {
        return true;
    }
    return false;
}

bool es_comando_planificable (char* comando) {
    return (es_string(comando, "CREATE") OR es_string(comando, "INSERT") OR es_string(comando, "SELECT") OR es_string(comando, "DROP") OR es_string(comando, "RUN") OR es_string(comando, "DESCRIBE"));
}

bool es_string(char* string, char* comando) {
    return string_equals_ignore_case(string, comando);
}

void notificar_error_comando_incorrecto (void) {
    printf("Error: comando incorrecto.\n");
    log_info(logger, "Error: comando incorrecto.");
}

void notificar_error_comando_cantidad_parametros(void) {
    printf("Error: cantidad de parámetros incorrecta.\n");
    log_info(logger, "Error: cantidad de parámetros incorrecta.");
}

void notificar_error_tipo_consistencia(void) {
    printf("Error: criterio de consistencia incorrecto.\n");
    log_info(logger, "Error: criterio de consistencia incorrecto.");
}

void ejecutar_metricas(t_memoria_del_pool* memoria){
	printf("METRICAS DE LA MEMORIA %d\n", memoria->numero_memoria);
	printf("Memory Load: %d \n", memoria->cantidad_carga);
	printf("Writes: %d \n",memoria->cantidad_insert);
	printf("Write Latency / 30s: %f \n",memoria->tiempo_insert);
	printf("Reads: %d \n", memoria->cantidad_select);
	printf("Read Latency / 30s: %f\n", memoria->tiempo_select);
}

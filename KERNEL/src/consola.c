#include "consola.h"

#define ever ;;
#define OR ||
#define AND &&

void consola() {
    
    for(ever) {
        char* linea = console();

        if(es_string(linea, "EXIT")) {
            free(linea);
            break;
        }

        procesar_comando(linea);

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

                log_info(logger, "NO es un comando planificable.");
                if (es_string(parametros[0],"JOURNAL")) {

                	log_info(logger, "Ejecuto journal");
                	
                    //agregar un par de memorias a l_memorias
                    log_info(logger, "Memorias: %d.", list_size(l_memorias));
                    printf("Memorias: %d.\n", list_size(l_memorias));

                    //t_list* l_memorias_activas = list_create();
                    //l_memorias_activas = filtrar_memorias_activas();
                    t_list* l_memorias_activas = filtrar_memorias_activas();
                    
                    if (l_memorias_activas != NULL) {
                        //  Hay memorias activas
                        log_info(logger, "Memorias activas: %d.", list_size(l_memorias_activas));
                        printf("Memorias activas: %d.\n", list_size(l_memorias_activas));

                        //TODO: a cada memoria de la lista l_memorias_activas hay que mandarle una función (a hacer) para que cada una haga el JOURNAL...
                        //¿podría ser un list_iterate(l_memorias, dicha_funcion)?
                    }
                    else {
                        //  No hay memorias activas
                        log_info(logger, "No hay memorias activas para hacerles el journal.");
                        puts("No hay memorias activas para hacerles el journal.");
                    }

                    list_destroy_and_destroy_elements(l_memorias_activas, (void*) free_memoria);
                    
                }
				if (es_string(parametros[0], "ADD")) {
	                    
                    //  ADD MEMORY numero_de_memoria TO criterio_de_consistencia
                    t_memoria_del_pool* m = obtener_memoria(atoi(parametros[2]));
                    if (m != NULL) {
                        //  El número de memoria está en la lista de memorias
                        
                        if (es_string(parametros[4], "SC") OR es_string(parametros[4], "SHC") OR es_string(parametros[4], "EC")) {
                            
                            if (es_string(parametros[4], "SC")) {
                                
                                if (list_is_empty(l_criterio_SC)) {
                                	log_info(logger, "Se agrega a criterio SC memoria: %d", m->numero_memoria);
                                    list_add(l_criterio_SC, m );
                                }
                                else {
                                    log_info(logger, "Error en el comando ADD: Ya existe una memoria asignada al criterio SC.");
                                    puts("Error en el comando ADD: Ya existe una memoria asignada al criterio SC.");
                                }
                                
                            }
                            else if (es_string(parametros[4], "SHC")) {

                            	//TODO: verificar si hay elementos en la lista, si ya hay elementos ejecutar journal en cada memoria de esta lista. luego agregar la nueva memoria
                            	log_info(logger, "Se agrega a criterio SHC memoria: %d", m->numero_memoria);
                                list_add(l_criterio_SHC, m );
                            }
                            else {
                                log_info(logger, "Se agrega a criterio EC memoria: %d", m->numero_memoria );
                                list_add(l_criterio_EC, m );
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
                        puts("Error en el comando ADD: número de memoria desconocido.");
                    }

				}
				if (es_string(parametros[0], "METRICS")) {

					log_info(logger, "Ejecuto metrics");
					//TODO: ejecutar metrics
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
    list_add(l_pcb_nuevos, pcb);
    
    //  Se mueve el PCB de la lista de NEW a la lista de READY
    list_add(l_pcb_listos, list_remove(l_pcb_nuevos, 0));

    log_info(logger, "Se crea el PCB de la request: %s con id: %d ", pcb->request_comando , pcb->id);
    log_info(logger, "Cantida de pcb listos: %d ", list_size( l_pcb_listos ));
    id_pcbs++;
}

t_list* filtrar_memorias_activas (void) {
    t_list* m = NULL;
    
    bool _es_memoria_activa (t_memoria_del_pool *x) {
        return x->activa;
    }

    if (!list_is_empty(l_memorias)) {
        m = list_filter(l_memorias, (void*) _es_memoria_activa);
    }

    return m;
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
    return (es_string(comando, "CREATE") OR es_string(comando, "INSERT") OR es_string(comando, "SELECT") OR es_string(comando, "DROP") OR es_string(comando, "RUN") OR es_string(comando, "DESCRIBE") );
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


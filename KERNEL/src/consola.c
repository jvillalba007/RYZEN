#include "consola.h"

#define ever ;;
#define OR ||
#define AND &&

void consola() {
    
    for(ever) {
        char* linea = console();

        if(es_comando(linea, "EXIT")) {
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
                if (es_comando(parametros[0], "RUN")) {
                    tipo = COMPUESTA;

                    log_info(logger, "Es un RUN.");
                }
                else {
                    tipo = SIMPLE;
                    log_info(logger, "Es un CREATE, un INSERT, un SELECT o un DROP.");
                }
                
                //  Se crea el PCB, se lo carga en la lista de NEW y se lo mueve a la lista de READY
                crear_pcb(linea, tipo);

            }
            else {

                log_info(logger, "NO es un comando planificable.");
                if( string_equals_ignore_case(parametros[0],"JOURNAL" ) ){

                	log_info(logger, "Ejecuto journal");
                	//TODO: ejecutar el journal
                }
				if( string_equals_ignore_case(parametros[0],"ADD" ) ){

					log_info(logger, "Ejecuto add");
					//TODO: ejecutar el ADD
				}
				if( string_equals_ignore_case(parametros[0],"METRICS" ) ){

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

bool es_comando_conocido (char** parametros) {
    if (es_comando(parametros[0], "CREATE") OR
    es_comando(parametros[0], "INSERT") OR
    es_comando(parametros[0], "SELECT") OR
    es_comando(parametros[0], "DROP") OR
    es_comando(parametros[0], "RUN") OR
    es_comando(parametros[0], "DESCRIBE")OR
    es_comando(parametros[0], "JOURNAL") OR
    (   
        es_comando(parametros[0], "ADD") AND
        es_comando(parametros[1], "MEMORY") AND
        es_comando(parametros[3], "TO")
    ) OR
    es_comando(parametros[0], "METRICS") OR
    es_comando(parametros[0], "CLEAR")) {
        return true;
    }
    else {
        return false;
    }
}

bool es_correcta_cantidad_parametros (char* comando, int cantidad) {
    if (es_comando(comando, "CREATE") AND (cantidad == 5)) {
        return true;
    }
    else if (es_comando(comando, "INSERT") AND (cantidad == 4)) {
        return true;
    }
    else if (es_comando(comando, "SELECT") AND (cantidad == 3)) {
        return true;
    }
    else if (es_comando(comando, "DROP") AND (cantidad == 2)) {
        return true;
    }
    else if (es_comando(comando, "RUN") AND (cantidad == 2)) {
        return true;
    }
    else if (es_comando(comando, "DESCRIBE") AND ((cantidad == 1) OR (cantidad = 2))) {
        return true;
    }
    else if (es_comando(comando, "JOURNAL") AND (cantidad == 1)) {
        return true;
    }
    else if (es_comando(comando, "ADD") AND (cantidad == 5)) {
        return true;
    }
    else if (es_comando(comando, "METRICS") AND (cantidad == 1)) {
        return true;
    }
    else if (es_comando(comando, "CLEAR") AND (cantidad == 1)) {
        return true;
    }
}

bool es_comando_planificable (char* comando) {
    return (es_comando(comando, "CREATE") OR es_comando(comando, "INSERT") OR es_comando(comando, "SELECT") OR es_comando(comando, "DROP") OR es_comando(comando, "RUN") OR es_comando(comando, "DESCRIBE") );
}

bool es_comando(char* string, char* comando) {
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


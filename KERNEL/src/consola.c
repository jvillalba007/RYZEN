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

        log_info(logger, "Es un comando conocido.");
        
        //  Se verifica si la cantidad de parámetros es correcta
        if (es_correcta_cantidad_parametros(parametros[0], cant_parametros)) {

            log_info(logger, "Es correcta la cantidad de parámetros.");

            //  Se verifica si el comando es "planificable"
            if (es_comando_planificable(parametros[0])) {

                log_info(logger, "Es un comando planificable.");
                
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

                //  Hago otra cosa...

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

void crear_pcb (char* string_codigo, t_tipo_request tipo) { //devolvería un void o un int (la id)
    t_PCB* pcb = malloc(sizeof(t_PCB));
    pcb->id = id_pcbs;
    pcb->request_comando = strdup(string_codigo);
    pcb->pc = 1;
    pcb->tipo_request = tipo;  
    
    //  Se crea el PCB
    list_add(l_pcb_nuevos, pcb);
    
    //  Se mueve el PCB de la lista de NEW a la lista de READY
    list_add(l_pcb_listos, list_remove(l_pcb_nuevos, 1));

    log_info(logger, "Se crea el PCB de la request: %s con id: %d ", pcb->request_comando , pcb->id);
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
    return (es_comando(comando, "CREATE") OR es_comando(comando, "INSERT") OR es_comando(comando, "SELECT") OR es_comando(comando, "DROP") OR es_comando(comando, "RUN"));
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



//  PARA EL HILO DE EJECUCIÓN:

/* SELECT
    Ejemplo: SELECT nombre_de_la_tabla key
    
    nombre_de_la_tabla = parametros[1];
    key = paramentros[2];

    criterio_de_consistencia_asociado = obtener_criterio_consistencia_tabla(nombre_de_la_tabla);

    if (strcmp(criterio_de_consistencia_asociado, "SC")) {
        memoria_ip = obtener_ip_memoria_de_lista_pool_memorias(numero_memoria_con_criterio_SC);
        memoria_puerto = obtener_puerto_memoria_de_lista_pool_memorias(numero_memoria_con_criterio_SC);
    }
    else if (strcmp(criterio_de_consistencia_asociado, "SHC")) {
        memoria_ip = obtener_ip_memoria_de_lista_pool_memorias(funcion_hash(key, cantidad_memorias_actuales_pool_de_memorias));
        memoria_puerto = obtener_puerto_memoria_de_lista_pool_memorias(funcion_hash(key, cantidad_memorias_actuales_pool_de_memorias));
    }
    else if (strcmp(criterio_de_consistencia_asociado, "EC")) {
        memoria_ip = obtener_ip_memoria_de_lista_pool_memorias(funcion_hash(key, cantidad_memorias_actuales_pool_de_memorias));
        memoria_puerto = obtener_puerto_memoria_de_lista_pool_memorias(funcion_hash(key, cantidad_memorias_actuales_pool_de_memorias));
    }

    delegar_select_a_memoria(memoria_ip, memoria_puerto, nombre_de_la_tabla, key);

    loggear();
    recibir_resultados_de_memoria();
    mostrar_resultados();
*/

/* INSERT
    Ejemplo: INSERT nombre_de_la_tabla key value
    
    nombre_de_la_tabla = parametros[1];
    key = parametros[2];
    value = parametros[3];

    criterio_de_consistencia_asociado = obtener_criterio_consistencia_tabla(nombre_de_la_tabla);

    if (strcmp(criterio_de_consistencia_asociado, "SC")) {
        memoria_ip = obtener_ip_memoria_de_lista_pool_memorias(numero_memoria_con_criterio_SC);
        memoria_puerto = obtener_puerto_memoria_de_lista_pool_memorias(numero_memoria_con_criterio_SC);
    }
    else if (strcmp(criterio_de_consistencia_asociado, "SHC")) {
        memoria_ip = obtener_ip_memoria_de_lista_pool_memorias(funcion_hash(key, cantidad_memorias_actuales_pool_de_memorias));
        memoria_puerto = obtener_puerto_memoria_de_lista_pool_memorias(funcion_hash(key, cantidad_memorias_actuales_pool_de_memorias));
    }
    else if (strcmp(criterio_de_consistencia_asociado, "EC")) {
        memoria_ip = obtener_ip_memoria_de_lista_pool_memorias(funcion_hash(key, cantidad_memorias_actuales_pool_de_memorias));
        memoria_puerto = obtener_puerto_memoria_de_lista_pool_memorias(funcion_hash(key, cantidad_memorias_actuales_pool_de_memorias));
    }
    
    delegar_insert_a_memoria(memoria_ip, memoria_puerto, nombre_de_la_tabla, key, value);

    loggear();
    recibir_resultados_de_memoria();
    mostrar_resultados();
*/

/* CREATE
    Ejemplo: CREATE nombre_de_la_tabla tipo_de_consistencia cantidad_de_particiones tiempo_de_compactacion
            
    nombre_de_la_tabla = parametros[1];
    tipo_de_consistencia = parametros[2];
    cantidad_de_particiones = parametros[3];
    tiempo_de_compactacion = parametros[4];

    Se delega DIRECTAMENTE a la memoria del archivo de configuración.
    delegar_create(memoria_ip, memoria_puerto, nombre_de_la_tabla, tipo_de_consistencia, cantidad_de_particiones, tiempo_de_compactacion);

    loggear();
    recibir_resultados_de_memoria();
    mostrar_resultados();
*/

/* DESCRIBE
if (cantParametros == 1) {
    Ejemplo (único): DESCRIBE
    
    Se delega DIRECTAMENTE a la memoria del archivo de configuración.
    delegar_describe_global(memoria_ip, memoria_puerto);    

    loggear();
    recibir_resultados_de_memoria();
    mostrar_resultados();       
}
else
//if (cantParametros == 2) {
    Ejemplo: DESCRIBE nombre_de_la_tabla

    nombre_de_la_tabla = parametros[1];

    Se delega DIRECTAMENTE a la memoria del archivo de configuración.
    delegar_describe_1_tabla(memoria_ip, memoria_puerto, nombre_de_la_tabla); 

    loggear();
    recibir_resultados_de_memoria();
    mostrar_resultados();
}
*/
//string_equals_ignore_case
/* DROP
Ejemplo: DROP nombre_de_la_tabla

    nombre_de_la_tabla = parametros[1];
    int* nombre_de_la_tabla = parametros[1];
    printf("Se DROPea la tabla %s.\n", nombre_de_la_tabla);

    Se delega DIRECTAMENTE a la memoria del archivo de configuración.
    delegar_drop(memoria_ip, memoria_puerto, nombre_de_la_tabla); 

    loggear();
    recibir_resultados_de_memoria();
    mostrar_resultados();
*/

/* ADD
    Ejemplo: ADD MEMORY numero_de_memoria TO tipo_de_consistencia
    int numero_de_memoria = atoi(parametros[2]);
    char* tipo_de_consistencia = parametros[4];

//  PRUEBA_DEL_BARRA_N clean_new_line_ch
limpiar_caracter_final_de_nueva_linea(tipo_de_consistencia);
//  FIN PRUEBA

//TODO refactor de esto, SC tambien sera una lista no se va a guardar en un int
if (string_equals_ignore_case(tipo_de_consistencia, "SC")) {
    //numero_memoria_con_criterio_SC = numero_de_memoria;
    printf("Se agrega la memoria %d al criterio SC.\n", numero_de_memoria);

    //  ...se labura acá...
    //numero_memoria_con_criterio_SC = numero_de_memoria;

    //printf("Número de memoria con criterio SC: %d\n", numero_memoria_con_criterio_SC);

}
else 
    if (string_equals_ignore_case(tipo_de_consistencia, "SHC")) {
        printf("Se agrega la memoria %d al criterio SHC.\n", numero_de_memoria);

        //  ...se labura acá...

    }
    else {
        if (string_equals_ignore_case(tipo_de_consistencia, "EC")) {
        printf("Se agrega la memoria %d al criterio EC.\n", numero_de_memoria);
        
        //  ...se labura acá...

        }
        else {
            notificar_error_tipo_consistencia();
        }
    }
*/

/* RUN
if (cantParametros == 2) {
    //  Ejemplo: RUN ruta_del_archivo_LQL
    
    char* ruta_del_archivo_LQL = parametros[1];
    FILE *archivo = NULL;
    archivo = fopen(ruta_del_archivo_LQL, "r");

    if (archivo == NULL) {
        puts("Archivo no encontrado.");
    }
    
    size_t buffer_size = 100;
    char *lineaLeida = malloc(buffer_size * sizeof(char));

    int numero_de_linea = 0;
    while (getline(&lineaLeida, &buffer_size, archivo) != -1) {
        if (strcmp(lineaLeida, "\n")) printf("%s", lineaLeida);
        
        procesar_comando(lineaLeida);

    }
    fflush(stdout);
    fclose(archivo);
    free(lineaLeida);          
    
    /*   
    else {
        while (!feof(archivo))
        {
            fgets(lineaLeida, 100, archivo);
            if (strcmp(lineaLeida, "\n") != 0) { //el archivo no debe terminar con un "\n". Si no, falla (duplica la última línea leída).
                printf("%s", lineaLeida);

                procesar_comando(lineaLeida);
                //  y todo lo demás...

                /* Al ingresar por consola RUN 2.LQL, se muestra lo siguiente
                            ADD MEMORY 1 TO SC
                NO -------> Error: criterio de consistencia incorrecto.
                            ADD MEMORY 2 TO SHC
                NO -------> Error: criterio de consistencia incorrecto.
                            ADD MEMORY 3 TO EC
                NO -------> Error: criterio de consistencia incorrecto.
                            ADD MEMORY 4 TO SC
                NO -------> Error: criterio de consistencia incorrecto.
                CASI -----> ADD MEMORY 5 TO SHCSe agrega la memoria 5 al criterio SHC
                ? --------> .

                En los primeros 4 casos:
                Me parece que el último parámetro guarda el "\n" (¿será culpa de fgets? eso)... por eso falla al compararlo con "SC", o con "SHC" o con "EC".
                En el último caso, cuando no hay "\n", anda bien digamos...


                Por otro lado: si se pone un archivo que no existe, el programa muestra "Ruta del archivo incorrecta" y termina.
                Hay que corregirlo...
                
            }
        }
        printf("\n");
    }
    
    fclose(archivo);
*/

/* CLEAR
//  Comando "clear" para limpiar la pantalla de la consola
if (cantParametros == 1) {
    system("clear");
}
else {
    notificar_error_sintactico_en_parametros();
}
*/




//  PARA EL HILO QUE HACE EL DESCRIBE GLOBAL CADA CIERTO TIEMPO

/*
unsigned long long obtener_tiempo_actual (void) { //revisar por qué no reconoce el "tv" que en las pruebas del Eclipse sí anda...
    struct timeval tv;
	unsigned long long milisegundos_unix_epoch;
    gettimeofday(&tv, NULL);
    milisegundos_unix_epoch = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
    return milisegundos_unix_epoch;
}
*/





//  Otros...
//  PRUEBA_DEL_BARRA_N clean_new_line_ch
/*void limpiar_caracter_final_de_nueva_linea(char *linea) {
    int posicion_nueva_linea = strlen(linea) - 1;
    if (linea[posicion_nueva_linea] == '\n') linea[posicion_nueva_linea] = '\0';
}*/
//  FIN PRUEBA

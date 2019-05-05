#include "consola.h"

#define ever ;;

void consola() {
    for(ever) {
        char* linea = console();

        //Esto se "podría" hacer así también: char* linea = console("Kernel"); ... habría que hacerlo...

        if(string_equals_ignore_case(linea,"SALIR")) {
                free(linea);
                break;
        }

        procesar_comando(linea);

        free(linea);
    }
    exit(EXIT_SUCCESS);
}

void procesar_comando(char* linea) {
    char** parametros = string_split(linea, " ");
	int cantParametros = split_cant_elem(parametros);

    //  En los siguientes 6 casos, hay que redireccionar a la memoria elegida por el criterio de la tabla en cuestión (pasada por parámetro: parametros[1])...
    if (string_equals_ignore_case(parametros[0], "SELECT")) {
        if (cantParametros == 3) {
            //  Ejemplo: SELECT nombre_de_la_tabla key

            //  nombre_de_la_tabla = parametros[1];
            //  key = paramentros[2];

            //  Delegar al módulo Memoria.
        }
        else {
            notificar_error_sintactico_en_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "INSERT")) {
        if (cantParametros == 4) {
            //  Ejemplo: INSERT nombre_de_la_tabla key value

            //  nombre_de_la_tabla = parametros[1];
            //  key = parametros[2];
            //  value = parametros[3];

            //  Delegar al módulo Memoria.
        }
        else {
            notificar_error_sintactico_en_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "CREATE")) {
        if (cantParametros == 5) {
            //  Ejemplo: CREATE nombre_de_la_tabla tipo_de_consistencia cantidad_de_particiones tiempo_de_compactacion
            
            //  nombre_de_la_tabla = parametros[1];
            //  tipo_de_consistencia = parametros[2];
            //  cantidad_de_particiones = parametros[3];
            //  tiempo_de_compactacion = parametros[4];

            //  Delegar al módulo Memoria.
        }
        else {
            notificar_error_sintactico_en_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "DESCRIBE")) {
        if (cantParametros == 1) {
            //  Ejemplo (único): DESCRIBE
            
            //  Delegar al módulo Memoria.
            
        }
        else
        if (cantParametros == 2) {
            //  Ejemplo: DESCRIBE nombre_de_la_tabla

            //  nombre_de_la_tabla = parametros[1];

            //  Delegar al módulo Memoria.
        }
        else {
            notificar_error_sintactico_en_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "DROP")) {
        if (cantParametros == 2) {
            //  Ejemplo: DROP nombre_de_la_tabla

            //  nombre_de_la_tabla = parametros[1];
            
            //  Delegar al módulo Memoria.
        }
        else {
            notificar_error_sintactico_en_parametros();
        }
    }    
    else

    //  Los siguientes 4 casos son tareas que realiza el Kernel en particular...

    if (string_equals_ignore_case(parametros[0], "JOURNAL")) {
        if (cantParametros == 1) {
            //  Ejemplo: JOURNAL
            //  No tiene otros parámetros

            //  ...laburar acá...
        }
        else {
            notificar_error_sintactico_en_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "ADD") && string_equals_ignore_case(parametros[1], "MEMORY") && string_equals_ignore_case(parametros[3], "TO")) {
        if (cantParametros == 5) {
            //  Ejemplo: ADD MEMORY numero_de_memoria TO tipo_de_consistencia
            
            //  numero_de_memoria = parametros[2];
            //  tipo_de_consistencia = parametros[4];

            //  ...laburar acá...
        }
        else {
            notificar_error_sintactico_en_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "RUN")) {
        if (cantParametros == 2) {
            //  Ejemplo: RUN ruta_del_archivo_LQL
            
            //  ruta_del_archivo_LQL = parametros[1];

            //  ...laburar acá...
        }
        else {
            notificar_error_sintactico_en_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "METRICS")) {
        if (cantParametros == 1) {
            //  Ejemplo: METRICS 
            //  No tiene otros parámetros

            //  ...laburar acá...
        }
        else {
            notificar_error_sintactico_en_parametros();
        }
    }
    else

    //  Comando "clear" para limpiar la pantalla de la consola
    if (string_equals_ignore_case(parametros[0], "CLEAR")) {
        if (cantParametros == 1) {
            system("clear");
        }
        else {
            notificar_error_sintactico_en_parametros();
        }
    }
    
    //  Si no se ingresa ningún comando...
    else {
        notificar_error_sintactico_en_comando();
    }

	split_liberar(parametros);
}

void notificar_error_sintactico_en_comando(void) {
    puts("Error sintáctico al ingresar el comando.");
}

void notificar_error_sintactico_en_parametros(void) {
    puts("Error sintácico: la cantidad de parámetros es incorrecta.");
}
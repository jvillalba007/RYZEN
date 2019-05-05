#include "consola.h"

#define ever ;;

void consola() {
    for(ever) {
        //ORIGINAL
        //char* linea = console();

        //PRUEBA
        char* linea = console("Kernel");


        //BIEN HECHO DEBERÍA SER ASÍ:
        //char* linea = console("Kernel");

        //ASÍ NO ANDA, PICHÓN: char* linea = readline("Kernel > ");

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

    //  En los siguientes 6 casos, hay que redireccionar a la memoria elegida por el criterio de la tabla en cuestión (pasada por parámetro: parametro[1])...
    if (string_equals_ignore_case(parametros[0], "SELECT")) {
        if (cantParametros == 3) {
            //  Traído de MEMORIA:
            //  string_iterate_lines(parametros,puts);
        }
        else {
            notificar_error_sintactico_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "INSERT")) {
        if (cantParametros == 4) {
            //  Delegar al módulo Memoria.
            
            /*  Traído de MEMORIA:
            char** parametros_aux = string_split(linea, "\""); //spliteo la linea con comillas
            char** parametros_estaticos  = string_split(parametros_aux[0], " "); //obtiene INSERT,[TABLA],[KEY]
            string_iterate_lines(parametros_estaticos,puts);
            puts(parametros_aux[1]); //parametros_aux[1] esta el "VALUE"
            split_liberar(parametros_estaticos);
            split_liberar(parametros_aux);
            */
        }
        else {
            notificar_error_sintactico_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "CREATE")) {
        if (cantParametros == 5) {
            //  Delegar al módulo Memoria.

            /*  Traído de MEMORIA:
            string_iterate_lines(parametros,puts);
            */
        }
        else {
            notificar_error_sintactico_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "DESCRIBE")) {
        if (cantParametros == 1) {
            //  Delegar al módulo Memoria.

            /*  Traído de MEMORIA:
            string_iterate_lines(parametros,puts);
            */
        }
        else
        if (cantParametros == 2) {
            //  Delegar al módulo Memoria.

            /*  Traído de MEMORIA:
            string_iterate_lines(parametros,puts);
            */
        }
        else {
            notificar_error_sintactico_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "DROP")) {
        if (cantParametros == 2) {
            //  Delegar al módulo Memoria.

            /*  Traído de MEMORIA:
            string_iterate_lines(parametros,puts);
            */
        }
        else {
            notificar_error_sintactico_parametros();
        }
    }    
    else

    //  Los siguientes 4 casos son tareas que realiza el Kernel en particular...

    if (string_equals_ignore_case(parametros[0], "JOURNAL")) {
        if (cantParametros == 1) {
            //  Ejemplo: JOURNAL
            //  No tiene otros parámetros

            //
        }
        else {
            notificar_error_sintactico_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "ADD") && string_equals_ignore_case(parametros[1], "MEMORY") && string_equals_ignore_case(parametros[3], "TO")) {
        if (cantParametros == 5) {
            //  Ejemplo: ADD MEMORY numero_de_memoria TO tipo_de_consistencia
            //  numero_de_memoria = parametro[2];
            //  tipo_de_consistencia = parametro[4];

            //
        }
        else {
            notificar_error_sintactico_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "RUN")) {
        if (cantParametros == 2) {
            //  Ejemplo: RUN ruta_del_archivo_LQL
            //  ruta_del_archivo_LQL = parametro[1];

            //
        }
        else {
            notificar_error_sintactico_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "METRICS")) {
        if (cantParametros == 1) {
            //  METRICS

            //
        }
        else {
            notificar_error_sintactico_parametros();
        }
    }
    else

    //  Comando "clear" para limpiar la pantalla de la consola
    if (string_equals_ignore_case(parametros[0], "CLEAR")) {
        if (cantParametros == 1) {
            system("clear");
        }
        else {
            notificar_error_sintactico_parametros();
        }
    }
    
    //  Si no se ingresa ningún comando...
    else {
        notificar_error_sintactico_comando();
    }

	split_liberar(parametros);
}

void notificar_error_sintactico_comando(void) {
    puts("Error sintáctico al ingresar el comando.");
}

void notificar_error_sintactico_parametros(void) {
    puts("Error sintácico: la cantidad de parámetros es incorrecta.");
}
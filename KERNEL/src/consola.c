#include "consola.h"

#define ever ;;
#define OR ||
#define AND &&

void consola() {
    for(ever) {
        char* linea = console();

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
    if (string_equals_ignore_case(parametros[0], "ADD") AND string_equals_ignore_case(parametros[1], "MEMORY") AND string_equals_ignore_case(parametros[3], "TO")) {
        if (cantParametros == 5) {
            //  Ejemplo: ADD MEMORY numero_de_memoria TO tipo_de_consistencia
            int numero_de_memoria = atoi(parametros[2]);
            char* tipo_de_consistencia = parametros[4];

            if (string_equals_ignore_case(tipo_de_consistencia, "SC")) {
                printf("Se agrega la memoria %d al criterio SC\n.", numero_de_memoria);

                //  ...se labura acá...

            }
            else 
                if (string_equals_ignore_case(tipo_de_consistencia, "SHC")) {
                    printf("Se agrega la memoria %d al criterio SHC\n.", numero_de_memoria);

                    //  ...se labura acá...

                }
                else {
                    if (string_equals_ignore_case(tipo_de_consistencia, "EC")) {
                    printf("Se agrega la memoria %d al criterio EC\n.", numero_de_memoria);
                    
                    //  ...se labura acá...

                    }
                    else {
                        notificar_error_tipo_consistencia();
                    }
                }
        }
        else {
            notificar_error_sintactico_en_parametros();
        }
    }
    else
    if (string_equals_ignore_case(parametros[0], "RUN")) {
        if (cantParametros == 2) {
            //  Ejemplo: RUN ruta_del_archivo_LQL
            
            char* ruta_del_archivo_LQL = parametros[1];
            char* lineaLeida = (char*) malloc(sizeof(char) * 100);
            
            FILE *archivo = NULL;
            archivo = fopen(ruta_del_archivo_LQL, "r");
            if(archivo == NULL ) {
                puts("Ruta del archivo incorrecta.");
            }
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
                        */
                    }
                }
                printf("\n");
            }
            
            fclose(archivo);
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
    puts("Error sintáctico: comando incorrecto.");
}

void notificar_error_sintactico_en_parametros(void) {
    puts("Error sintácico: cantidad de parámetros incorrecta.");
}

void notificar_error_tipo_consistencia(void) {
    puts("Error: criterio de consistencia incorrecto.");
}
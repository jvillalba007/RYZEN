#include "utils.h"

void split_liberar(char** split){
	unsigned int i = 0;
	for(; split[i] != NULL;i++){
		free(split[i]);
	}
	free(split);
}

int split_cant_elem(char** split){
	int i = 0;
	for(; split[i] != NULL; i++);
	return i;
}

char* string_extract_substring(char* s, char* pattern_start, char* pattern_end){

    char *target = NULL;
    char *start, *end;

    if ( (start = strstr( s, pattern_start )) )
    {
        start += strlen( pattern_start );
        if ( (end = strstr( start, pattern_end )) )
        {
            target = ( char * )malloc( end - start + 1 );
            memcpy( target, start, end - start );
            target[end - start] = '\0';
        }
    }

    return target;

}

void remove_substring (char *string, char *sub) {
    char *match = string;
    int len = strlen(sub);
    while ((match = strstr(match, sub))) {
        *match = '\0';
        strcat(string, match+len);
                match++;
    }
}

void remove_value(char* linea, char* value)
{
	int bytes = strlen(value)+3;
	char* value_comillas = calloc(bytes,sizeof(char));
	strcat(value_comillas,"\"" );
	strcat(value_comillas,value);
	strcat(value_comillas,"\"" );

	remove_substring (linea, value_comillas);
	free(value_comillas);
}

void limpiar_caracter_final_de_nueva_linea (char *linea) {
    int posicion_nueva_linea = strlen(linea) - 1;
    if (linea[posicion_nueva_linea] == '\n') linea[posicion_nueva_linea] = '\0';
}

char* generate_path(char* table_name, char* table_folder, char* file_extension) {
	char* full_path = (char *)calloc(strlen(table_name) + strlen(table_folder) + strlen(file_extension) + 1, sizeof(char));
	strcat(full_path, table_folder);
	strcat(full_path, table_name);
	strcat(full_path, file_extension);

	return full_path;
}


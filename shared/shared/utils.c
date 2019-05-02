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
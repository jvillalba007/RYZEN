#include "console.h"

size_t bufsize = 32;


size_t console(char *buffer) {


    size_t characters;


    if( buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }

    printf("> ");
    characters = getline(buffer,&bufsize,stdin);
    //printf("%zu characters were read.\n",characters);

    return characters;

}


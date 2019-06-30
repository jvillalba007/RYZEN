/*
 * dump.c
 *
 *  Created on: 17 jun. 2019
 *      Author: utnso
 */

#include "LFS.h"

char* pathTemporal;
char* _tmp;

void dump_registros(fila_registros* registro)
{
	int ok;
	char *data = string_from_format("%" PRIu64 ";%d;%s\n",registro->timestamp,registro->key,registro->value);
	guardarDatos(pathTemporal,strlen(data),data,&ok);

	free(data);
	free(registro->value);
	free(registro);
}

void obtenerArchivoTemp(char* table_name)
{
    DIR *d;
    struct dirent *dir;
	char* rutaTemporal = string_new();
	string_append(&rutaTemporal, "tables/");
	string_append(&rutaTemporal, table_name);

	int tmp = 0;

	char * table_path = generate_path(rutaTemporal, "", "");

    d = opendir(table_path);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if(string_ends_with(dir->d_name,".tmp"))
            {
            	char* valorTMP = string_substring(dir->d_name, 0 , strlen(dir->d_name) - 4);
            	tmp = max(tmp,atoi(valorTMP));
            	free(valorTMP);
            }
         }
        closedir(d);
    }

    free(rutaTemporal);
    free(table_path);

    _tmp = string_from_format("%d.tmp",tmp+1);
}

void obtenerPathTemporal(fila_memtable* ftabla) {
	char* rutaTemporal = string_new();
	string_append(&rutaTemporal, "tables/");
	string_append(&rutaTemporal, ftabla->tabla);
	string_append(&rutaTemporal, "/");
	string_append(&rutaTemporal, _tmp);
	pathTemporal = generate_path(rutaTemporal, "", "");
	free(rutaTemporal);
}

void dumpear_tablas(fila_memtable* ftabla){
		int ok;
		obtenerArchivoTemp(ftabla->tabla);
		crearTemporal(ftabla->tabla, _tmp, &ok);

		if(ok != -1)
		{
		obtenerPathTemporal(ftabla);
		list_iterate(ftabla->registros, (void*) dump_registros);
		list_destroy(ftabla->registros);
		log_info(g_logger, "DUMP DE REGISTROS");
		free(ftabla->tabla);
		free(_tmp);
		free(pathTemporal);
		free(ftabla);
		log_info(g_logger, "DUMP DE TABLA");
		log_info(g_logger, "LIBERADO TABLA de memtable");
		}
		else
		{
			liberador_registros(ftabla);
			log_error(g_logger, "NO SE PUEDE HACER DUMP DE TABLA");
			log_error(g_logger, "LIBERADO TABLA de memtable");
		}

}

void dumpear_tablas_memtable() {

		if(!list_is_empty(memtable))
		{
			list_iterate(memtable,(void*)dumpear_tablas);
			list_destroy(memtable);
			log_info(g_logger, "LIBERADO MEMTABLE");
			memtable = list_create();
			log_info(g_logger, "NUEVO MEMTABLE");
		}
}

void dump()
{
    struct timespec ts;
    ts.tv_sec = lfs_config.tiempo_dump / 1000;
    ts.tv_nsec = (lfs_config.tiempo_dump  % 1000) * 1000000;

    assignHandler();

	while ( !EXIT_PROGRAM ) {

	    nanosleep(&ts, NULL);
	    dumpear_tablas_memtable();

	}

	pthread_exit(0);
}


#include "memoria.h"

int main(void) {
	if (mem_initialize() == -1) {
		mem_exit();
		return EXIT_FAILURE;
	}

	imprimir_config();

	liberar_mem_config(mem_config);
	mem_exit();

	return EXIT_SUCCESS;
}

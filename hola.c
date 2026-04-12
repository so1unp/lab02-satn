/*
 * Imprime un saludo por la salida estándar.
 */

#include <stdio.h>   // printf()
#include <stdlib.h>  // exit(), EXIT_SUCCESS

int main(int argc, char *argv[]) {
    if (argc > 1) {
        //char *msg = argv[1];
		printf("Hola %s\n", argv[1]);
	}	else {
		printf("Hola desconocido\n");
	}
	exit(EXIT_SUCCESS);
}

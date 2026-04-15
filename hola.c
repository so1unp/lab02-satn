/*
 * Imprime un saludo por la salida estándar.
 */

#include <stdio.h>
#include <stdlib.h>     // exit()

int main(int argc, char *argv[])
{
    if (argc > 1) {
        printf("Hola %s\n", argv[1]);  // Imprime el saludo.
    }
    
    // Termina la ejecución del proceso.
    exit(EXIT_SUCCESS);
}

/*
El sistema invoca la siguiente llamada: write(1, "Hola francisco\n", 15) = 15
Hace la llamada de write para que el sistema muestre en pantalla el mensaje,
donde el primer parámetro es el descriptor de archivo, donde va a mostrar (1 = salida estándar), 
el segundo es el buffer con lo que va a mostrar,
y el tercero es la cantidad de bytes que se van a escribir.
El resultado = 15 muestra que se escribieron bien todos los bytes.
*/
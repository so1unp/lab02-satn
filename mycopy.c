/*
 * Copia un archivo en otro.
 */

#include <stdlib.h> // exit()
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "No se pasaron los 4 parámetros necesarios.\n");
        exit(EXIT_FAILURE);
    }

    if (argv[1][0] != 's' && argv[1][0] != 'f')
    {
        fprintf(stderr, "No se pasó como primer parámetro ni 's' ni 'f'.\n");
        exit(EXIT_FAILURE);
    }

    char modo = argv[1][0];

    if (modo == 's')
    {
        int fd_origen = open(argv[2], O_RDONLY);

        if (fd_origen == -1)
        {
            perror("Error al abrir el archivo de origen");
            exit(EXIT_FAILURE);
        }

        int fd_destino = open(argv[3], O_WRONLY | O_CREAT | O_EXCL, 0644);

        if (fd_destino == -1)
        {
            perror("El archivo de destino no se pudo crear");
            exit(EXIT_FAILURE);
        }

        char buffer[1024];

        int bytes_leidos = read(fd_origen, buffer, sizeof(buffer));
        while (bytes_leidos > 0)
        {
            write(fd_destino, buffer, bytes_leidos);
            bytes_leidos = read(fd_origen, buffer, sizeof(buffer));
        }

        close(fd_origen);
        close(fd_destino);
    }
    else if (modo == 'f')
    {
        FILE *f_origen = fopen(argv[2], "r");
        if(f_origen == NULL) {
            perror("Error al intentar abrir el archivo de origen");
            exit(EXIT_FAILURE);
        }

        FILE *f_destino = fopen(argv[3], "wx");
        if(f_destino == NULL) {
            perror("Error al intentar abrir el archivo de destino");
            exit(EXIT_FAILURE);
        }

        char buffer[1024];

        size_t bytes_leidos = fread(buffer, 1, sizeof(buffer), f_origen);
        while (bytes_leidos > 0)
        {
            fwrite(buffer, 1, bytes_leidos, f_destino);
            bytes_leidos = fread(buffer, 1, sizeof(buffer), f_origen);
        }

        fclose(f_origen);
        fclose(f_destino);
    }

    // Termina la ejecución del proceso.
    exit(EXIT_SUCCESS);
}
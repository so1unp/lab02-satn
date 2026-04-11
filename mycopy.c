/*
 * Copia un archivo en otro.
 */

#include <stdlib.h>     // exit()
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 1
#define MODE_PARAM_LENGTH 1

int copySyscall(char *from, char*to);
int copyStdio(char *from, char*to);
int validateParams(int paramsCount,char*params[]);

int main(int argc, char *argv[])
{
    char *mode = argv[1];
    char *from = argv[2];
    char *to = argv[3];

    if(validateParams(argc,argv) < 0){
        perror("Error parametro corrupto");
        exit(EXIT_FAILURE);
    }

    if(strlen(mode) != MODE_PARAM_LENGTH){
        perror("Error parametro corrupto");
        exit(EXIT_FAILURE);
    }

    if(mode[0] == 's'){
        if(copySyscall(from,to) < 0){
            perror("Error no se pudo copiar el archivo");
            exit(EXIT_FAILURE);
        }
    } else if(mode[0] == 'f') {
        if(copyStdio(from,to) < 0){
            perror("Error no se pudo copiar el archivo");
            exit(EXIT_FAILURE);
        }
    }

    // Termina la ejecución del proceso.
    exit(EXIT_SUCCESS);
}

int copySyscall(char *from, char*to) {
    int fdFrom;
    int fdTo;
    int bytesRead;
    char buffer[BUFFER_SIZE];

    if((fdFrom = open(from,O_RDONLY)) < 0){
        perror("Error el archivo origen no existe");
        return - 1;
    }
    if(open(to,O_RDONLY) > 0){
        perror("Error el archivo destion ya existe");
        return - 1;
    }
    fdTo = open(to,O_CREAT | O_WRONLY, 0644);
    while((bytesRead = read(fdFrom,buffer,BUFFER_SIZE)) > 0){
        if(write(fdTo,buffer,bytesRead) < 0 ){
            perror("Error al escribir el archivo");
            return - 1;
        }
    }
    close(fdFrom);
    close(fdTo);
    return 0;
}

int copyStdio(char *from, char*to){
    FILE *fpFrom;
    FILE *fpTo;
    char buffer[BUFFER_SIZE];
    int bytesRead;

    fpFrom = fopen(from,"r");
    if(fpFrom == NULL){
        perror("Error el archivo origen no existe");
        return -1;
    }
    
    fpTo = fopen(to,"r");

    if(fpTo != NULL){
        perror("Error el archivo destino ya existe");
        return -1;
    }

    fpTo = fopen(to,"w");

    while((bytesRead = fread(buffer,BUFFER_SIZE,BUFFER_SIZE,fpFrom)) > 0) {
        if(fwrite(buffer,BUFFER_SIZE,BUFFER_SIZE,fpTo) < 0 ){
            perror("Error al escribir el archivo");
            return - 1;
        }
    }

    return 0;
}

int validateParams(int paramsCount,char*params[]) {
    if(paramsCount < 0) {
        perror("El contador de parametros no puede ser negativo");
        return -1;
    }
    for(int i = 0; i < paramsCount; i++) {
        if(strlen(params[i]) <= 0) { 
            perror("Error el parametro svino vacio");
            return -1;
        }
    }
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

typedef struct strucRegistro {
    int estado;
    char descripcion[100];
} registro;

typedef struct msgbuf {
    long tipo;
    char data[156];
} mensaje;

void handle_int(int s);
void extraerMensaje();

int pid;
int ins;
char descr[100];
int cola;
int fd;

mensaje m;
registro r;


int main(int argc, char **argv) {
    // hago que con ctrl c se borre la cola.
    signal(SIGINT, (void*) handle_int);
    // creo cola de mensaje
    cola = msgget(0xa, IPC_CREAT | 0777);
    if (cola == -1) {
        printf("Error de creacion de cola de mensajes\n");
        return -1;
    }

    // existe el archivo?
    if(access("registros", F_OK) == 0) {
        fd  = open("registros", O_RDWR);
    } else {
        fd  = open("registros", O_CREAT | O_TRUNC | O_RDWR | 0666);
        // escribo los 1000 registros nulos.
        r.estado = 0;
        memset(r.descripcion, 0, 100);
        for(int i = 1000; i < 1000; i++) {
            write(fd, &r, sizeof(registro));
        }

    }

    // atiendo cola
    while(1) {
        // servidor siempre lee tipo 1;
        memset(m.data, 0, 156);
        msgrcv(cola, &m, 156, 1, 0);
        printf("Recibio: %s\n", m.data);
        


        extraerMensaje();

        printf("pid: %i\n", pid);
        printf("ins: %i\n", ins);
        printf("descr: %s\n", descr);
    
        return 1;
    }
}

void extraerMensaje() {
    printf("procesando: %s\n", m.data);

    // extraigo las disintas partes del mensaje y las guardo en las variables.
    char* token;
    char *delimitadores = ",";
    
    token = strtok(m.data, delimitadores);
    if(token != NULL) {
        printf("Token pid: %s\n", token);
        pid = atoi(token);
    }

    token = strtok(NULL, delimitadores);
    if(token != NULL) {
        printf("Token ins: %s\n", token);
        ins = atoi(token);
    }

    token = strtok(NULL, delimitadores);
    if(token != NULL) {
        printf("Token descr: %s\n", token);
        strcpy(descr, token);
    }

    return;
}

void handle_int(int s) {
    msgctl(cola, IPC_RMID, 0);
    close(fd);
}
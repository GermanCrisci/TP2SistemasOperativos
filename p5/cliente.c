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

void extraerMensaje(int* pid, int* ins, char** descr, mensaje* m);

int cola;
int pid;


int main(int argc, char **argv) {
    pid = getpid();
    // hago que con ctrl c se borre la cola.
    // creo cola de mensaje
    cola = msgget(0xa, IPC_CREAT | 0777);
    if (cola == -1) {
        printf("Error de creacion de cola de mensajes\n");
        return -1;
    }

    mensaje m;
    m.tipo = 1;
    char texto[130];
    // atiendo cola
    while(1) {
        memset(m.data, 0, 156);

        // cliente envia de tipo 1 con su pid
        printf("%i>", pid);
        fgets(texto, 130, stdin);
        texto[strlen(texto)-1] = '\0';
        sprintf(m.data, "%i,%s", pid, texto);
        printf("Enviando: %s\n", m.data);
        m.tipo = 1;
        msgsnd(cola, &m, 156, 0);

        
        // espero respuesta
        printf("Esperando respuesta...\n");
        msgrcv(cola, &m, 156, pid, 0);
        printf("Recibio: %s\n", m.data);

        /*
        int pid;
        int ins;
        char descr[100];
        char** tmp = (char**) &descr;

        extraerMensaje(&pid, &ins, tmp, &m);
        printf("pid: %i\n", pid);
        printf("ins: %i\n", ins);
        printf("ins: %s\n", descr);
        */
    }
}

void extraerMensaje(int* pid, int* ins, char** descr, mensaje* m) {
    printf("procesando: %s\n", (*m).data);

}
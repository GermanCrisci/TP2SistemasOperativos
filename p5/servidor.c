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

typedef struct strucRegistro
{
    int estado;
    char descripcion[100];
} registro;

typedef struct msgbuf
{
    long tipo;
    char data[156];
} mensaje;

void handle_int(int s);
void extraerMensaje();
void atenderCliente();
int buscarEspacio();

int pid;
int ins;
char descr[100];
int cola;
int fd;

mensaje m;
registro r;

int main(int argc, char **argv) {
    // hago que con ctrl c se borre la cola.
    signal(SIGINT, (void *)handle_int);
    // creo cola de mensaje
    cola = msgget(0xa, IPC_CREAT | 0777);
    if (cola == -1) {
        printf("Error de creacion de cola de mensajes\n");
        return -1;
    }

    // existe el archivo?
    if (access("registros", F_OK) == 0) {
        fd = open("registros", O_RDWR);
    }
    else {
        fd = open("registros", O_CREAT | O_TRUNC | O_RDWR | 0666);
        // escribo los 1000 registros nulos.
        r.estado = 0;
        strcpy(r.descripcion, "");
        for (int i = 0; i < 1000; i++)
        {
            write(fd, &r, sizeof(r));
        }
    }

    // atiendo cola
    memset(m.data, 0, 156);
    // servidor siempre lee tipo 1;
    while (msgrcv(cola, &m, 156, 1, 0)) {
        printf("Recibio: %s\n", m.data);

        extraerMensaje();

        /*
        printf("pid: %i\n", pid);
        printf("ins: %i\n", ins);
        printf("descr: %s\n", descr);
        */

        atenderCliente();

        memset(m.data, 0, 156);
    }
}

void enviarError(char error[100])
{
    sprintf(m.data, "Error: %s", error);
    msgsnd(cola, &m, 156, 0);
}

void atenderCliente()
{
    // ignoro interrupcion ctrl c hasta que termine de atender
    sigset_t set;
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);

    // atiendo
    // creacion FUNCIONA BIEN
    if (ins == -1) { 
        // agrego nuevo en el primer espacio libre.
        int num = buscarEspacio();
        if(num >= 0) {
            lseek(fd, sizeof(r)*num, SEEK_SET);
            r.estado = 1;
            strcpy(r.descripcion, descr);
            write(fd, &r, sizeof(registro));
            // envio confirmacion
            m.tipo = pid;
            sprintf(m.data, "1,%i,Se creo el registro exitosamente", num);
            msgsnd(cola, &m, 156, 0);
        } else { // no habia espacio
            m.tipo = pid;
            strcpy(m.data, "0,No hay espacio para mas registros");
            msgsnd(cola, &m, 156, 0);
        }
    } else {
        if (strncmp(descr, "leer", 4) == 0 || strncmp(descr, "borrar", 6) == 0) {
            if (ins > 1000 || ins < 0)
                enviarError("Se envio un numero de registro invalido.");

            // lectura FUNCION BIEN
            if (strncmp(descr, "leer", 4) == 0 && ins > 0 && ins <= 1000) {
                // leo el registro, en ins me llego el numero
                lseek(fd, sizeof(r) * (ins - 1), SEEK_SET);
                read(fd, &r, sizeof(r));
                m.tipo = pid;
                if (r.estado == 0 || r.estado == 2) { // caso no existe
                    sprintf(m.data, "0,%i,El registro que desea no existe", ins);
                    msgsnd(cola, &m, 156, 0);
                } else { // caso existe
                    sprintf(m.data, "1,%i,%s", ins, r.descripcion);
                    msgsnd(cola, &m, 156, 0);
                }
            }

            // borrado FUNCIONA BIEN
            if (strncmp(descr, "borrar", 6) == 0 && ins > 0 && ins <= 1000) {
                // leo el registro, en ins me llego el numero
                lseek(fd, sizeof(r) * (ins - 1), SEEK_SET);
                m.tipo = pid;
                if (r.estado == 1) {
                    // pongo estado borrado.
                    r.estado = 2;
                    // modifico el registro
                    write(fd, &r, sizeof(r));
                    // envio confirmacion
                    sprintf(m.data, "1,%i,Se elimino el registro %i exitosamente", ins);
                    msgsnd(cola, &m, 156, 0);
                } else {
                    sprintf(m.data, "0,%i,No existe el registro que intento eliminar", ins);
                    msgsnd(cola, &m, 156, 0);
                }
            }
        } else {
            // modificacion FUNCION BIEN
            if (ins > 0 && ins <= 1000) {
                // leo el registro, en ins me llego el numero
                lseek(fd, sizeof(r) * (ins - 1), SEEK_SET);
                r.estado = 1;
                strcpy(r.descripcion, descr);
                write(fd, &r, sizeof(r));
                // envio confirmacion
                m.tipo = pid;
                sprintf(m.data, "Se modifico el registro %i exitosamente", ins);
                msgsnd(cola, &m, 156, 0);
            }
        }
    }

    sigprocmask(SIG_UNBLOCK, &set, NULL);
}

int buscarEspacio() {
    // leo los registros hasta que encuentro un espacio vacio.
    registro r2;
    // voy al principio del archivo.
    lseek(fd, 0, SEEK_SET);
    int seEncontro = 0;
    int num = 0;
    while(read(fd, &r, sizeof(r))) {
        if(r.estado == 0 || r.estado == 2)
            return num;
        num++;
    }
    return -1;
}

void extraerMensaje()
{
    printf("procesando: %s\n", m.data);

    // extraigo las disintas partes del mensaje y las guardo en las variables.
    char *token;
    char *delimitadores = ",";

    token = strtok(m.data, delimitadores);
    if (token != NULL) {
        printf("Token pid: %s\n", token);
        pid = atoi(token);
    }

    token = strtok(NULL, delimitadores);
    if (token != NULL) {
        printf("Token ins: %s\n", token);
        ins = atoi(token);
    }

    token = strtok(NULL, delimitadores);
    if (token != NULL) {
        printf("Token descr: %s\n", token);
        strcpy(descr, token);
    }

    return;
}

void handle_int(int s)
{
    msgctl(cola, IPC_RMID, 0);
    close(fd);
    raise(SIGKILL);
}
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
    int bloqueado; //=0(libre), <>0(bloqueado, contiene el pid del proceso que lo bloquea)
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
        r.bloqueado = 0;
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
        int num = buscarEspacio() + 1;
        if(num > 0) {
        	lseek(fd, sizeof(r)*(num-1), SEEK_SET);
            read(fd, &r, sizeof(r));
        	if(r.bloqueado == 0 || r.bloqueado == pid){
            	r.estado = 1;
            	strcpy(r.descripcion, descr);
            	lseek(fd, sizeof(r)*(num-1), SEEK_SET);
            	write(fd, &r, sizeof(registro));
            	// envio confirmacion
            	m.tipo = pid;
            	sprintf(m.data, "1,%i,Se creo el registro exitosamente", num);
            	msgsnd(cola, &m, 156, 0);
            } else {
            	m.tipo = pid;
            	sprintf(m.data, "0,%i,Registro bloqueado por proceso %i", num, r.bloqueado);
            	msgsnd(cola, &m, 156, 0);
            }
        } else { // no habia espacio
            m.tipo = pid;
            strcpy(m.data, "0,No hay espacio para mas registros");
            msgsnd(cola, &m, 156, 0);
        }
    } else {
        if (strncmp(descr, "leer", 4) == 0 || strncmp(descr, "borrar", 6) == 0 || strncmp(descr, "lock", 4) == 0 || strncmp(descr, "unlock", 6) == 0) {
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
                	if (r.bloqueado == 0 || r.bloqueado == pid){
                    	sprintf(m.data, "1,%i,%s", ins, r.descripcion);
                    	msgsnd(cola, &m, 156, 0);
                    } else {
                    	sprintf(m.data, "0,%i,Registro bloqueado por %i", ins, r.bloqueado);
                    	msgsnd(cola, &m, 156, 0);
                    }
                }
            }

            // borrado FUNCIONA BIEN
            if (strncmp(descr, "borrar", 6) == 0 && ins > 0 && ins <= 1000) {
                // leo el registro, en ins me llego el numero
                lseek(fd, sizeof(r) * (ins - 1), SEEK_SET);
                read(fd, &r, sizeof(r));
                m.tipo = pid;
                if (r.estado == 1) {
                    // pongo estado borrado.
                    r.estado = 2;
                    // modifico el registro
                    lseek(fd, sizeof(r) * (ins - 1), SEEK_SET);
                    write(fd, &r, sizeof(r));
                    // envio confirmacion
                    sprintf(m.data, "1,%i,Se elimino el registro %i exitosamente", ins, ins);
                    msgsnd(cola, &m, 156, 0);
                } else {
                    sprintf(m.data, "0,%i,No existe el registro que intento eliminar", ins);
                    msgsnd(cola, &m, 156, 0);
                }
            }
            //Lock
            if (strncmp(descr, "lock", 4) == 0 && ins > 0 && ins <= 1000) {
            	lseek(fd, sizeof(r)*(ins-1),SEEK_SET);
            	read(fd, &r, sizeof(r));
            	m.tipo = pid;
            	if(r.bloqueado == 0) {
            		r.bloqueado = pid;
            		lseek(fd, sizeof(r)*(ins-1),SEEK_SET);
            		write(fd, &r, sizeof(r));
            		sprintf(m.data,"1,%i, Se bloqueo correctamente el registro",ins);
            		msgsnd(cola, &m, 156, 0);
            	}
            	else {
            		sprintf(m.data,"0,%i, Registro ya bloqeuado por %i",ins, r.bloqueado);
            		msgsnd(cola, &m, 156, 0);
            	}
            }
            //Unlock
            if (strncmp(descr, "unlock", 6) == 0 && ins > 0 && ins <= 1000) {
            	lseek(fd, sizeof(r)*(ins-1),SEEK_SET);
            	read(fd, &r, sizeof(r));
            	m.tipo = pid;
            	if(r.bloqueado == pid) {
            		r.bloqueado = 0;
            		lseek(fd, sizeof(r)*(ins-1),SEEK_SET);
            		write(fd, &r, sizeof(r));
            		sprintf(m.data,"1,%i, Se desbloqueo correctamente el registro",ins);
            		msgsnd(cola, &m, 156, 0);
            	} else {
            		if(r.bloqueado ==0){
            			sprintf(m.data,"0,%i, El registro ya estaba desbloqueado",ins);
            			msgsnd(cola, &m, 156, 0);
            		} else {
            			sprintf(m.data,"0,%i, Registro bloqueado por %i, sigue bloqueado",ins, r.bloqueado);
            			msgsnd(cola, &m, 156, 0);
            		}
            	}
            }
        } else {
            // modificacion FUNCION BIEN
            if (ins > 0 && ins <= 1000) {
                // leo el registro, en ins me llego el numero
                lseek(fd, sizeof(r) * (ins - 1), SEEK_SET);
                read(fd, &r, sizeof(r));
                m.tipo = pid;
                if(r.bloqueado == 0 || r.bloqueado == pid){
                	r.estado = 1;
                	strcpy(r.descripcion, descr);
                	lseek(fd, sizeof(r) * (ins - 1), SEEK_SET);
                	write(fd, &r, sizeof(r));
                	// envio confirmacion
                	sprintf(m.data, "1,%i,Se modifico el registro %i exitosamente", ins, ins);
                	msgsnd(cola, &m, 156, 0);
                } else {
                	sprintf(m.data, "0,%i,Registo bloqueado por %i", ins, r.bloqueado);
                	msgsnd(cola, &m, 156, 0);
                }
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

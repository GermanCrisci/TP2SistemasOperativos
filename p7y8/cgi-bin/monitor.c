#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sem.h>

int *ptrSHM = NULL;
FILE *fdHtml = NULL;
int espacio;
int direccionSHM;
char *nombreHtml;
int tiempoRecarga = 0;
char contenidoHtml[1024 * 1024];
int shmid;
char cmdargs[2048], args[1024], cmd[1024], msg[300];
struct stat st;
int direccionSem = 0xa;

void handle_alarma(int sig);
void handle_int(int sig);

int main(int argc, char *argv[])
{

    signal(SIGINT, handle_int);
    // en argumentos llega
    /**
     * 1 - archivo html a cargar
     * 2 - cant en segundos cuando se recarga en memoria el archivo html
     * 3 - tamanio en KB inicial de memoria compartida
     * 4 - clave hexadecimal de memoria compartida donde ser carga el archivo
     */

    nombreHtml = argv[1];
    printf("Archivo a monitorear: '%s'\n", nombreHtml);

    tiempoRecarga = atol(argv[2]);
    if (tiempoRecarga <= 0)
    {
        printf("Error: Por favor ingrese un tiempo de recarga valido\n");
        return -1;
    }
    printf("El tiempo de recarga en segundos es: '%X'\n", tiempoRecarga);

    espacio = atol(argv[3]) * 1024;
    if (espacio <= 0)
    {
        printf("Error: Por favor ingrese un espacio en valido\n");
        return -1 ;
    }
    printf("El espacio en SHM en KB es: '%i'\n", espacio);

    direccionSHM = (int)strtol(argv[4], NULL, 16);
    if (direccionSHM <= 0)
    {
        printf("Error: Por favor ingrese una direccion de SHM valida\n");
        return -1;
    }
    printf("La direccion de SHM es: '%X'\n", direccionSHM);

    printf("Validando existencia de archivo...\n");
    if ((fdHtml = fopen(nombreHtml, "r")))
    {
        printf("Archivo valido!\n");
    }
    else
    {
        printf("Error: Archivo ingresado no existe!\n");
        return -1;
    }

    printf("Entraria el archivo en memoria compartida?\n");
    stat(nombreHtml, &st);
    printf("Largo de archivo en Bytes %ld\n", st.st_size);
    if (st.st_size > (espacio))
    {
        printf("Error: No entraria el HTML en memoria!\n");
        return -1;
    }
    else
    {
        printf("Espacio en memoria compartida suficiente!\n");
    }

    printf("Creando espacio en memoria compartida...\n");
    shmid = shmget(direccionSHM, espacio, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        printf("Error: Hubo un error al crear el espacio en memoria compartida! %i\n", shmid);
        return -1;
    }
    else
    {
        printf("Espacio en memoria creado exitosamente, shmid = '%i'\n", shmid);
    }

    printf("Leyendo archivo...\n");
    fread(&contenidoHtml, st.st_size + 1, 1, fdHtml);
    contenidoHtml[st.st_size] = '\0';
    // printf("%s\n", contenidoHtml);

    printf("Guardando archivo en memoria...\n");
    int *ptrSHM = (int *) shmat(shmid, 0, 0);
    int i = 0;

    for (; i <= st.st_size; i++)
    {
        *(ptrSHM + i) = contenidoHtml[i];
        printf("%c", *(ptrSHM+i));
    }
    printf("Contenido guardado en memoria con exito!\n");

    // creando alarma para actualizacion
    printf("Creando alarma para actualizacion\n");
    signal(SIGALRM, handle_alarma);
    alarm(tiempoRecarga);
    fclose(fdHtml);
    while (1)
        pause();
    // borro el segmento de memoria compartida para testing
    return 1;
}

void handle_alarma(int sig)
{
    // esto busca el contenido del archivo otra vez y lo carga en la memoria
    printf("Actualizando datos en memoria...\n");
    printf("Validando existencia de archivo...\n");
    if ((fdHtml = fopen(nombreHtml, "r")))
    {
        printf("Archivo valido!\n");
    }
    else
    {
        printf("Error: Archivo ingresado no existe!\n");
        return;
    }

    if (st.st_size > (espacio))
    {
        printf("Error: El archivo fue modificado y ya no entra en memoria!\n");
        printf("Creando memoria devuelta con suficiente espacio!\n");
        shmid = shmget(direccionSHM, st.st_size * 2, IPC_CREAT | 0666);
        espacio = st.st_size * 2 / 1024;
        if (shmid < 0)
        {
            printf("Error: Hubo un error al crear el espacio en memoria compartida! %i\n", shmid);
            return;
        }
        else
        {
            printf("Espacio en memoria creado exitosamente, shmid = '%i'\n", shmid);
        }
    }

    fread(&contenidoHtml, st.st_size + 1, 1, fdHtml);
    contenidoHtml[st.st_size] = '\0';

    int *ptrSHM = (int *) shmat(shmid, 0, 0);
    int i = 0;
    printf("Guardando archivo en memoria... error aca\n");
    // NO SE PORQUE EL SEMAFORO NO FRENA EL CGI...
    for (; i <= st.st_size; i++)
    {
        *(ptrSHM + i) = contenidoHtml[i];
        printf("%c", *(ptrSHM + i));
    }
    printf("Contenido guardado en memoria con exito!\n");
    alarm(tiempoRecarga);
}

void handle_int(int sig)
{
    printf("\nEliminando memoria compartida y semaforo!\n");
    shmctl(shmid, IPC_RMID, 0);
    exit(0);
}
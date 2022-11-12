#define _GNU_SOURCE // habilita funciones como strcasestr() etc

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
#include <string.h>
#include <ctype.h>
#include <sys/sem.h>

// el archivo requiere que le cambies los permisos con
// sudo chmod a+x ver_html.cgi, si no te tira error 500 la request
// debe ser algo del lighttpd

int extraer(char *destino, int largo, const char *tag, char *buffer);
int esHexa(char c);
char charHexa(char *p);
int obtenercgi(char *destino, int largo);
void mostrarHTML(int *pSHM, char tag[256]);
void P(int, int);
void V(int, int);

char cmdargs[2048], args[1024], cmd[1024], msg[300];
char tag[256];
int semid;

int main()
{
    int shmid = shmget(0xa, 0, 0);
    if (shmid < 0)
    {
        printf("Hubo un error al conectar a la memoria compartida!\n");
        return -1;
    }
    else
    {
        // printf("Se conecto a la memoria compartida con exito, shmid = %i!\n", shmid);
    }

    semid = semget(0xa, 0, 0);
    if (semid < 0) {
        printf("Error: Hubo un error al conseguir el semaforo! %i\n", semid);
        return -1;
    }

    int *pSHM = (int *)shmat(shmid, 0, 0);

    memset(cmdargs, 0, 2048);
    obtenercgi(cmdargs, 2048);
    // printf("obtenercgi retorna %d metodo [%s]\n", rc, getenv("REQUEST_METHOD"));
    extraer(tag, 1024, "tag", cmdargs);
    printf("Se extraera contenido de tag: '%s'<br>\n", tag);
    mostrarHTML(pSHM, tag);
    /*
    int i;
    while(*(pSHM+i) != '\0') {
        printf("%c",*(pSHM+i));
        i++;
    }
    */
    printf("\n");
    return 1;
}

void mostrarHTML(int *pSHM, char tag[256])
{
    // printf("El tag tiene tamanio '%i'<br>\n", strlen(tag));
    int i1 = 0;
    int icomptag = 0;
    int i2 = 0;
    int mostrar = 0;
    // DEBERIA FRENARSE ACA, YA QUE EL MONITOR ESTA MODIFICANDO
    P(semid,0);
    while (*(pSHM + i1) != '\0')
    {
        if (*(pSHM + i1) == '<')
        {
            int compTag = true;
            if (*(pSHM + (i1 + 1)) == '/')
            {
                for (icomptag = 0; icomptag < strlen(tag); icomptag++)
                {
                    if (*(pSHM + (i1 + 2) + icomptag) != tag[icomptag])
                        compTag = false;
                }
                if (compTag)
                    break;
            }

            for (icomptag = 0; icomptag < strlen(tag); icomptag++)
            {
                if (*(pSHM + (i1 + 1) + icomptag) != tag[icomptag])
                    compTag = false;
            }
            if (compTag)
            {
                mostrar = 1;
                i1 += strlen(tag) + 2;
            }
        }

        if (mostrar)
        {
            printf("%c", *(pSHM + i1));
            i2++;
        }
        i1++;
    }
    V(semid,0);
    printf("\n");
    return;
}

int extraer(char *destino, int largo, const char *tag, char *buffer)
{
    char *p = strcasestr(buffer, tag), *out = destino;
    int n = 0;
    if (p)
    {
        p += strlen(tag) + 1;
        if (*p)
        {
            while (*p && *p != '&' && n < largo)
            {
                if (*p == '\n' || *p == '\r' || *p == '\t')
                {
                    p++;
                    continue;
                }
                if (*p == '+')
                {
                    *out = ' ';
                }
                else
                {
                    // posible caracter codificado %00 .. %FF
                    if (*p == '%' && esHexa(*(p + 1)) && esHexa(*(p + 2)))
                    {
                        *out = charHexa(p);
                        p += 2;
                    }
                    else
                    {
                        *out = *p;
                    }
                }
                out++;
                p++;
                n++;
            }
            *out = '\0';
            return 1;
        }
    }
    return 0;
}

/**
 * Devuelve 1 (verdad) si c es un digito hexadecimal
 * Sino devuelve 0 (falso)
 */
int esHexa(char c)
{
    char c2 = (char)toupper(c);
    if (c2 >= '0' && c2 <= '9')
        return 1;
    if (c2 >= 'A' && c2 <= 'F')
        return 1;
    return 0;
}
/**
 * p apunta a "%2A..." por ejemplo, esta funcion devuelve
 * el caracter ascii del valor hexa 2A
 */
char charHexa(char *p)
{
    char hexa[3];
    hexa[0] = *(p + 1);
    hexa[1] = *(p + 2);
    hexa[2] = '\0';
    return (char)strtol(hexa, NULL, 16);
}

int obtenercgi(char *destino, int largo)
{
    if (getenv("QUERY_STRING") == NULL)
        return 0;
    if (getenv("REQUEST_METHOD") == NULL)
        return 0;
    if (strcasecmp(getenv("REQUEST_METHOD"), "GET") == 0)
    { // es GET
        if (strlen(getenv("QUERY_STRING")) > largo)
        {
            // el contenido de query_string supera a largo!
            return 0;
        }
        strcpy(destino, getenv("QUERY_STRING"));
    }
    else
    { // es POST
        int lei = read(STDIN_FILENO, destino, largo - 1);
        if (lei >= 0)
            destino[lei] = '\0';
    }
    return 1;
}

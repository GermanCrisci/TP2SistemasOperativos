#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>

#define BUFFERSIZE 5

int main(void) {
	// creo dos semaforos SVR4
	int semid1 = semget(0xA,2,IPC_CREAT|0777);
	printf("semid1 = %d\n",semid1);
	// creo dos semaforos SVR4
	int semid2 = semget(0xB,2,IPC_CREAT|0777);
	printf("semid2 = %d\n",semid2);

    if (access("chat1", F_OK) == 0) {
		remove("chat1");
    }
	if (access("chat2", F_OK) == 0) {
		remove("chat2");
    }

	// estos permisos si no se crea el archivo sin permisos
    int fd1 = open("chat1", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	printf("fd1=%i\n", fd1);
    int fd2 = open("chat2", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	printf("fd2=%i\n", fd2);

	// inicializo semaforos (ambos abiertos)
	/*
	./escritor 1 P(semid1,0) V(semid1,1)
	./escritor 2 P(semid2,0) V(semid2,1)
	./lector 1 P(semid1,1) V(semid2,0) 
	./lector 2 P(semid2,1) V(semid1,0)

    escritor 1 (escribe en archivo1)
    escritor 2 (escribe en archivo2)

    lector 1 (lee de archivo2)
    lector 2 (lee de archivo1)

	*/
	semctl(semid1,0,SETVAL,BUFFERSIZE); // 
	semctl(semid1,1,SETVAL,0); // lector 
	semctl(semid2,0,SETVAL,BUFFERSIZE); // SEMAFORO DEL ESCRITOR P()
	semctl(semid2,1,SETVAL,0); // lector 

	// desconecto memoria compartida
	exit(0);
}
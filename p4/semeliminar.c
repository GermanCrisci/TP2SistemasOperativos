#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>

int main(void) {
	// creo dos semaforos SVR4
	int semid1 = semget(0xA,0,0);
	printf("semid1 = %d\n",semid1);
	// creo dos semaforos SVR4
	int semid2 = semget(0xB,0,0);
	printf("semid2 = %d\n",semid2);

    remove("archivo1");
    remove("archivo2");

	semctl(semid1,0,IPC_RMID); 
	semctl(semid2,0,IPC_RMID); 

	// desconecto memoria compartida
	exit(0);
}
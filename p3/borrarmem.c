#include "memoria.h"

int main(int argc, char **argv)
{
	int shmid1 = shmget(0xA,0,0);
	printf("borro shmid1 = %d\n",shmid1);
	int semid1 = semget(0xA,0,0);
	printf("semid1 = %d\n",semid1);

	shmctl(shmid1,IPC_RMID,0);

	semctl(semid1,IPC_RMID,0);
	
	return 0;
}


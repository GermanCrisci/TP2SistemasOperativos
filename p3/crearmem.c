#include "memoria.h"

int main(int argc, char **argv)
{

	int shmid1 = shmget(0xA,sizeof(struct memchat),0777|IPC_CREAT);
	printf("shmid1 = %d\n",shmid1);

	int semid1 = semget(0xA,2,IPC_CREAT|0777);
	printf("semid1 = %d\n",semid1);

	struct memchat *m = (struct memchat *) shmat(shmid1,0,0);
	memset(m,0,sizeof(struct memchat));

	semctl(semid1,0,SETVAL,1); 
	semctl(semid1,1,SETVAL,1); 

	shmdt(m);
	
	return 0;
}


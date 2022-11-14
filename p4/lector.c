#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>



char mensaje[256];
int fd;
int id;
void P(int semid,int sem);
void V(int semid,int sem);
int main(int argc, char**argv){ 
	//if (atoi(argv[1])!=1 && atoi(argv[1])!=2){printf("EL id debe ser 1 o 2\n");return -1;}
	//if (atoi(argv[1]) == 1){id =1;}
	//if (atoi(argv[1]) == 2){id =0;}
	//creo semaforos
	int semid = semget(0xa,0,0);
	if (semid ==-1){printf("ERROR sem_get\n") ; exit(-1);}
	printf("Semaforos SVR4 creados\n");
	//inicializo semaforos
	//semctl(semid,0,SETVAL,1);
	//semctl(semid,1,SETVAL,1);

	if (access("filechat1", F_OK) == 0) {
        fd = open("filechat1", O_WRONLY);
        printf("Abro archivo\n");
    }
    else {
        fd = open("filechat1", O_CREAT | O_TRUNC | O_WRONLY | 0666);
        printf("Creo archivo\n");
	}
	//do {
			P(semid,id);
			lseek(fd,0,SEEK_SET);
			read(fd,&mensaje,256*sizeof(char));
			printf("%i<<%s\n",1,mensaje);		
			V(semid,id);
		//} while( strncmp(mensaje,"chau",4) != 0 );
	//semctl(semid, 0, IPC_RMID);
}

void P(int semid,int sem) {
	struct sembuf buf;
	buf.sem_num = sem;
	buf.sem_op = -1; 
	buf.sem_flg = 0;
	semop(semid,&buf,1);
}
void V(int semid,int sem) { 
	struct sembuf buf;
	buf.sem_num = sem;
	buf.sem_op = 1; 
	buf.sem_flg = 0;
	semop(semid,&buf,1);
}


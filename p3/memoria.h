#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define BUFFERSIZE 5  
struct memchat {
	int count[2];
	char buffer[2][BUFFERSIZE][256];  
} memchat;

void P(int semid,int sem); 
void V(int semid,int sem); 
void ingreso(char *prompt,char *dest,int size);

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


void ingreso(char *prompt,char *dest,int size) {
	printf("%s",prompt);
	fgets(dest,size,stdin);
	dest[strlen(dest)-1]='\0';
}

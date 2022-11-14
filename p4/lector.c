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

void P(int semid,int sem); // wait(S1)
// signal
void V(int semid,int sem); // signal(S0)
// ingreso
void ingreso(char *prompt,char *dest,int size);

int main(int argc,char **argv) {
	if ( argc != 2 ) {
		printf("./lector <nro>\n<nro> debe ser 1 o 2\n");
		return 2;
	}
	int id = atoi(argv[1]);
	printf("id=%d\n",id);
	if ( !(id >= 1 && id <= 2) ) {
		printf("<nro> indicado por linea de comandos debe ser 1 o 2\n");
		return 3;
	}

	/*
	./escritor 1 P(semid1,0) V(semid1,1)
	./escritor 2 P(semid2,0) V(semid2,1)
	./lector 1 P(semid1,1) V(semid2,0) 
	./lector 2 P(semid2,1) V(semid1,0)
	*/

	// habro el archivo al que leo
	int fd;
	if(id == 1) {
    	fd = open("chat2", O_RDWR);
	} else {
        fd = open("chat1", O_RDWR);
	}

	int semid1 = semget(0xA,0,0);
	printf("semid1=%d\n",semid1);
	int semid2 = semget(0xB,0,0);
	printf("semid2=%d\n",semid2);

	char buffer[256];
	char prompt[20];
	sprintf(prompt,"U%d<",id);
	int out = 0;		
	do {
		(id == 1) ? P(semid2,1) : P(semid1,1);
		// escribo en memoria compartida lo ingresado por teclado	
		lseek(fd, 256*out, SEEK_SET);
		read(fd,&buffer, 256);
		printf("%s%s\n",prompt,buffer);
		(id == 1) ? V(semid1,0) : V(semid2,0);
		out = (out + 1) % BUFFERSIZE;
	} while(strncmp(buffer,"chau",4) != 0);
	printf("lector fin!\n");	
	// desconecto detach de memoria compartida
	exit(0);
}

// wait
void P(int semid,int sem) { // wait(S1)
	struct sembuf buf;
	buf.sem_num = sem;
	buf.sem_op = -1; // wait, resta 1
	buf.sem_flg = 0;
	semop(semid,&buf,1);
}

// signal
void V(int semid,int sem) { // signal(S0)
	struct sembuf buf;
	buf.sem_num = sem;
	buf.sem_op = 1; // signal, suma 1
	buf.sem_flg = 0;
	semop(semid,&buf,1);
}

// ingreso
void ingreso(char *prompt,char *dest,int size) {
	printf("%s",prompt);
	fgets(dest,size,stdin);
	dest[strlen(dest)-1]='\0';
}

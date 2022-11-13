//Para correr: ./chatmem <1 o 2>

#include "memoria.h"

int main(int argc, char **argv)
{
	if ( argc != 2 ) {
		printf("./chatmem <nro>\n<nro> debe ser 1 o 2\n");
		return 2;
	}
	
	int id = atoi(argv[1]);
	printf("id=%d\n",id);
	
	if ( !(id >= 1 && id <= 2) ) {
		printf("<nro> indicado por linea de comandos debe ser 1 o 2\n");
		return 3;
	}

	int shmid = shmget(0xA,0,0);
	int semid = semget(0xA,0,0);
	struct memchat *m = shmat(shmid,0,0);
	char buffer[256];
	char prompt[20];
	sprintf(prompt,"U%d>",id);
	
	int in = 0;	
	int out = 0;
	
	pid_t pid = fork();
	if ( pid == 0 ) {  //El hijo lee
		
		if (id == 1) {
			id = 2;
		} else id = 1;
				
		sprintf(prompt,"U%d>",id);
		
		do {
		
		while(m->count[id-1]==0);		
			
		strcpy(buffer,m->buffer[id-1][out]);
		printf("%s%s\n",prompt,buffer);
		P(semid,id-1); 
			m->count[id-1]--;
		V(semid,id-1); 
		out = (out + 1) % BUFFERSIZE;
		} while(strncmp(buffer,"chau",4) != 0);	
		printf("lector fin!\n");	
	
		shmdt(m);
		
		
	} else {  //El padre escribe
		
		do {
		while(m->count[id-1]>=BUFFERSIZE);  //espera activa
		
		ingreso(prompt,buffer,256);
		
		strcpy(m->buffer[id-1][in],buffer);
		P(semid,id-1); 
			m->count[id-1]++;					
		V(semid,id-1); 
		in = (in + 1) % BUFFERSIZE;
		} while(strncmp(buffer,"chau",4) != 0);  
		printf("escritor fin!\n");
		
		shmdt(m);
	
	
		wait(0); 
	}
	
	return 0;
}


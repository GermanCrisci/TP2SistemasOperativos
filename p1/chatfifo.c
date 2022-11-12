/*
 * chatfifo.c
 * 
 * crear los fifos desde la consola con:
 * 										mkfifo fifo1
 * 										mkfifo fifo2
 * 
 * Forma de ejecucion posible:
 * $ ./chat pepe fifo1 fifo2
 * $ ./chat juan fifo2 fifo1
 * 
 * Luego borrar los fifos desde consola con:
 * 											unlink fifo1
 * 											unlink fifo2
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

void enviarMsg(int fd,const char *);
void recibirMsg(int fd,char *dest,int size,const char *);

int main(int argc, char **argv)
{
	if ( argc != 4 ) {
		printf("Forma de uso:\n$ ./chat [nombre] [fifo escritura] [fifo lectura]\n");
		return 1;
	}
	
	char buffer[256];
	pid_t pid = fork();
	if ( pid == 0 ) {
		// lector 
		char lprompt[100];
		snprintf(lprompt,100,"%s>",argv[1]);
		int fd1 = open(argv[3],O_RDONLY);
		do {
			recibirMsg(fd1,buffer,256,lprompt);
		} while( strncmp(buffer,"chau",4) != 0 );
		close(fd1);
		printf("fin lector %s!\n",argv[1]);
	} else {
		// escritor
		int fd1 = open(argv[2],O_WRONLY);
		do {
			printf("%s>",argv[1]);
			fgets(buffer,256,stdin);
			enviarMsg(fd1,buffer);
		} while( strncmp(buffer,"chau",4) != 0 );
		close(fd1);  
		printf("fin escritor %s en wait!\n",argv[1]);
		wait(0); // espera x lector
	}	
		
	return 0;
}

void enviarMsg(int fd,const char *msg) {
	write(fd,msg,strlen(msg)+1);
	//printf("Envie %d bytes al proceso hijo!\n",bytes);
}

void recibirMsg(int fd,char *dest,int size,const char *prompt) {
	int bytes = read(fd,dest,size);
	if ( bytes > 0 ) {
		//system("clear"); 
		printf("recibio:%s",dest);
	}	
	else printf("No Recibi nada!\n");
}



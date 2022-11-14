#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

typedef struct msgbuf {
	long tipo;
	char data[256];
}mensaje;

int main (int argc, char **argv) {
	
	pid_t pid = fork();
	int msgid = msgget(0XA,0);
	mensaje m;
	
	if (argc != 3) {
		printf("Forma de uso:\n./chatcola <tipo escritor> <tipo lector>\n");
		return 1;
	}
	
	if (msgid == -1) {
		printf("Error al abrir la cola de mensajes\n");
		return 2;
	}
	
	if (pid != 0) {
		long tipo = atol(argv[1]);
		m.tipo = tipo;
		do{
			memset(m.data,0,256);
			printf("%li>",tipo);
			fgets(m.data,256,stdin);
			m.data[strlen(m.data)-1] = '\0';
			msgsnd(msgid,&m,256,0);
		} while (strcmp(m.data,"chau") != 0);
	wait(0);
	}else{
		memset(m.data,0,256);
		long tipo = atol(argv[2]);
		m.tipo = tipo;
		do {
		msgrcv(msgid,&m,256,tipo,0);
		printf("recibo:%s\n",m.data);
		} while (strcmp(m.data,"chau") != 0);
	}
}

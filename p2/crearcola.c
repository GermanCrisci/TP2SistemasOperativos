#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main(int argc, char **argv)
{
	int msgid = msgget(0XA,IPC_CREAT|0777);
	if (msgid == -1) {
		printf("Error al crear la cola de mensajes\n");
		return 2;
	}
	printf("Se ha creado la cola de mensajes\n");
	return 0;
}


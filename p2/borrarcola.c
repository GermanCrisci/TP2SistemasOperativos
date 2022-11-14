#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main(int argc, char **argv)
{
	int msgid = msgget(0XA,0);
	if (msgid == -1) {
		printf("Fallo la cola de mensajes\n");
		return 2;
	}
	msgctl(msgid,IPC_RMID,0);
	printf("Se destruyo la cola de mensajes\n");
	return 0;
}


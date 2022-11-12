#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

void P(int semid, int sem)
{
    struct sembuf buf;
    buf.sem_num = sem;
    buf.sem_op  = -1;
    buf.sem_flg = 0;
    semop(semid, &buf, 1);
}

void V(int semid, int sem)
{
    struct sembuf buf;
    buf.sem_num = sem;
    buf.sem_op  = 1;
    buf.sem_flg = 0;
    semop(semid, &buf, 1);
}
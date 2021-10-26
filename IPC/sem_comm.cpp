#include "sem_comm.h"

int commSemSet(int num, int flag)
{
    //获取一个信号量集的键值key
    key_t key = ftok(PATHNAME, PROJID);
    cout << "key : " << key << endl;
    if (key == -1)
    {
        perror("semget error 1");
        exit(1);
    }
    //获取信号量键关联的信号量标识
    int sem_id = semget(key, num, flag);
    if (sem_id == -1)
    {
        perror("semget error 2");
        exit(2);
    }
    return sem_id;
}

int CreateSemSet(int num)
{
    return commSemSet(num, IPC_CREAT | IPC_EXCL | 0666);
}

int DestroySemSet(int sem_id)
{
    int ret = semctl(sem_id, 0, IPC_RMID, 0);
    if (ret < 0)
    {
        perror("semctl error");
        return -1;
    }
    return 0;
}

int InitSem(int sem_id, int which)
{
    union semun un;
    un.val = 1;
    int ret = semctl(sem_id, which, SETVAL, un);
    if (ret < 0)
    {
        perror("semctl");
        return -1;
    }
    return 0;
}

int GetSemSet()
{
    return commSemSet(0, IPC_CREAT);
}

int SemOp(int sem_id, int op)
{
    struct sembuf buf = {0, op, SEM_UNDO};
    int ret = semop(sem_id, &buf, 1);
    if (ret < 0)
    {
        perror("semop error");
        return -1;
    }
    return 0;
}

int P(int sem_id)
{
    return SemOp(sem_id, -1);
}

int V(int sem_id)
{
    return SemOp(sem_id, 1);
}

int DestorySemSet(int sem_id)
{
    int ret = semctl(sem_id, 0, IPC_RMID);
    if (ret < 0)
    {
        perror("semctl error");
        return -1;
    }
    return 0;
}

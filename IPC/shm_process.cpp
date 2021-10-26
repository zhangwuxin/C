#include "shm_comm.h"
void testserver()
{
    int shm_id = CreatShm();
    printf("shm_id=%d\n", shm_id);
    char *mem = (char *)shmat(shm_id, NULL, 0);
    while (1)
    {
        sleep(1);
        printf("%s\n", mem);
    }
    shmdt(mem);
    DestroyShm(shm_id);
}
int main()
{
    testserver();
    return 0;
}

/*
g++ shm_comm.cpp shm_client.cpp -o shm_client -I ./ && g++ shm_process.cpp shm_comm.cpp -o shm_process -I ./ && ipcrm -M 0x66114042 ./shm_process
*/
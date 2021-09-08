#include <stdio.h>  
#include <sys/shm.h>  
#include <sys/ipc.h>  
#include <sys/sem.h>  
#include <string.h>  
#include <stdlib.h>  
#include <unistd.h>  
#define MY_SHM_ID 34325  
#define MY_SEM_ID 23234  
#define MAX_STRING 200  


typedef struct
{
    int semID;
    int counter;
    char string[MAX_STRING+1];
}MY_BLOCK_T;
int main(int argc, char** argv)
{
    int shmid, ret, i;
    MY_BLOCK_T* block;
    struct sembuf sb;
    char user;
    printf("argc : %d\n",argc);
    printf("argv1:%s\n",argv[1]);
    if (argc >= 2)
    {
        if(strcmp(argv[1],"create") == 0)
        {
            printf("create shared memory\n");
            shmid = shmget(MY_SHM_ID, sizeof(MY_BLOCK_T),(IPC_CREATE|0666));
            block = (MY_BLOCK_T*)shmat(shmid,(const void*)0,0);
            block -> counter = 0;
            block -> semID = semget(MY_SEM_ID,1,)
        }
    }
    return 0;
}

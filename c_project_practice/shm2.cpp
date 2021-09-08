#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#define MY_SHM_ID 67777


int main(){
    int shmid, ret;
    void* mem;
    shmid = shmget(MY_SHM_ID, 0, 0);
    if (shmid >= 0){
        mem = shmat(shmid, (const void*)0, 0);
        if ((int)mem != -1){
            printf("shared memory was attached in our address space at %p\n",mem);
            strcpy((char *)mem, "this is test.\n");
            printf("%s\n",(char*)mem);
            ret = shmdt(mem);
            if (ret == 0){
              printf("Success detached memory \n");
              printf("%s\n",(char*)mem);
            }
            else
              printf("Memory detached failed %d\n",errno);
        }else
          printf("shmat failed\n");
    }
    else
      printf("shared memory segment not found\n");
    return 0;
}

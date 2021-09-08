#include <stdio.h>
#include <unistd.h> // getpagesize();
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <stdlib.h>
#define MY_SHM_ID 67777



int main(){
    printf("1 %d \n",getpid());
    srand(getpid()*(time(NULL)%10000));
    printf("2 %d \n",getpid());
    printf("page size = %d\n",getpagesize());
    int shmid,ret;
    shmid = shmget(MY_SHM_ID,4096,0666|IPC_CREAT);
    if (shmid > 0)
        printf("Create a shared memory segment %d\n",shmid);
    struct shmid_ds shmds;
    ret = shmctl(shmid, IPC_STAT, &shmds);
    if (ret == 0 ){
        printf("size of memory segment is %d\n",shmds.shm_segsz);/*段的大小（以字节为单位）*/
        printf("number of attaches %d\n",(int)shmds.shm_nattch);/*当前附加到该段的进程的个数*/
    }
    else{
        printf ("shmctl call failed\n");
    }
    ret = shmctl(shmid, IPC_RMID, 0);
    if (ret == 0)
      printf ("shared memory removed\n");
    else
      printf("shared memory removed failed\n");
    return 0;
}

#include "sem_comm.h"


//函数功能 ： 使用信号量，完成父子进程的顺序打印
int main()
{
    //获取一个信号量集的标识
    int sem_id = CreateSemSet(1);
    //使用第0个
    InitSem(sem_id, 0);
    printf("sem_id : %d\n",sem_id);
    pid_t id = fork();
    int i = 10;
    if (id < 0)
    {
        perror("fork error");
        exit(1);
    }
    else if (id == 0){
        printf("child is running, pid = %d, ppid = %d \n",getpid(), getppid());
        usleep(100001);
        while(i)
        {
            P(sem_id);//P操作，信号量-1
            i--;
            printf("A\n");
            usleep(100001);
            fflush(stdout);
            printf("A\n");
            usleep(100004);
            fflush(stdout);
            V(sem_id);//V操作， 信号量+1
        }
    }else{
        printf("father is running, pid = %d, ppid = %d \n",getpid(), getppid());
        usleep(100001);
        while(i){
            P(sem_id);//P操作，信号量-1
            i--;
            printf("B\n");
            usleep(100003);
            fflush(stdout);
            printf("B\n");
            usleep(100005);
            fflush(stdout);
            V(sem_id);//V操作， 信号量+1
        }
        wait(NULL);
    }
    usleep(100001);
    DestroySemSet(sem_id);
    return 0;
}
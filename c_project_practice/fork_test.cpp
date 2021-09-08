#include <stdio.h>
#include <unistd.h>
#include <cstdlib>

int main(int argc, char *argv[])
{
    int i = 0;
    printf("before fork \n");
    printf("father pid :%d\n",getpid());
    pid_t pid = fork();
    printf("after fork \n");
    printf("pid : %d\n",(int)pid);
    
    if (pid < 0)
    {
        printf("error \b");
        return -1;
    }else if (pid == 0){
        printf("son pid :%d\n",getpid());
        while(1){
        }
        printf("son process \n");
        //while(i < 10){
        //    i++;
        //    printf("son i=%d\n",i);
        //    sleep(1);
        //}
        //exit(0);
    }else{
        printf("father process\n");
       // while(i<10){
        //    i+=2;
         //   printf("father i=%d\n",i);
         //   sleep(1);
        //}
        return 0;
    }
    return 0;
}


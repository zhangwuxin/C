#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/wait.h>
// #include "apue.h"
//匿名管道
//管道是IPC最基本的一种实现机制。我们都知道在Linux下“一切皆文件”，其实这里的管道就是一个文件。管道实现进程通信就是让两个进程都能访问该文件。
//它有一个读端一个写端，然后通过filedes参数传出给用户程序两个文件描述符，
//filedes[0]指向管道的读端，filedes[1]指向管道的写端（很好记，就像0是标准输入1是标准输出一样）。所以管道在用户程序看起来就像一个打开的文件，
//通过read(filedes[0]);或者write(filedes[1]);向这个文件读写数据其实是在读写内核缓冲区。pipe函数调用成功返回0，调用失败返回-1。

//函数功能：父进程向管道写入五次消息，子进程循环读取并
int main()
{
    int _pipe[2] = {0};
    int ret = pipe(_pipe);
    if (ret == -1)
    {
        perror("create pipe error!");
        return 1;
    }
    //_pipe[1] 用来写 _pipe[0] 用来读
    printf(
        "_pipe[0] is %d,
        _pipe[1] is %d\n",
         _pipe[0], _pipe[1]);
    pid_t id = fork();
    if (id < 0)
    {
        perror("fork error");
        return 2;
    }
    else if (id == 0)
    {
        printf("child writing\n");
        close(_pipe[0]);
        int count = 5;
        const char *msg = "I am From Father";
        while (count--)
        {
            write(_pipe[1], msg, strlen(msg));
            sleep(1);
        }
        close(_pipe[1]);
        exit(1);
    }
    else
    {
        printf("father reading\n");
        close(_pipe[1]);
        char msg[1024];
        int count = 5;
        while (count--)
        {
            size_t s = read(_pipe[0], msg, sizeof(msg) - 1);

            if (s > 0)
            {
                msg[s] = '\0';
                printf("child read :  # %s \n", msg);
            }
            else
            {
                perror("read error");
                exit(1);
            }
        }
        if (waitpid(id, NULL, 0) != -1)
        {
            printf("wait success\n");
        }
        return 0;
    }
}
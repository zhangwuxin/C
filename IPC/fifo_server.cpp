#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <unistd.h>
//命名管道
//对于 UNIX，有名管道为 FIFO。一旦创建，它们表现为文件系统的典型文件。
//通过系统调用 mkfifo()，可以创建 FIFO，通过系统调用 open()、read()、write()和close()，可以操作 FIFO

// 函数功能：生成myfifo命名管道文件，写入消息
int main(){
    int namepipe = mkfifo("myfifo", S_IFIFO|0666);
    if(namepipe == -1){
        perror("mkfifo error");
        exit(1);
    }
    int fd = open("./myfifo", O_RDWR);
    if(fd == -1){
        perror("open error");
        exit(2);
    }
    char buf[1024];
    while (1)
    {
        printf("sendto #");
        fflush(stdout);
        size_t s = read(0, buf, sizeof(buf)-1);
        if(s > 0){
            buf[s-1] = '\0';
            if(write(fd, buf, s) == -1){
                perror("write error");
                exit(3);
            }
        }
    }
    close(fd);
    return 0;
    
}
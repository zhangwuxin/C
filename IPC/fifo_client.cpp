#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <unistd.h>

// 函数功能：读取myfifo命名管道文件中的消息
int main(){
    int fd = open("./myfifo", O_RDWR);
    if(fd == -1){
        perror("open error");
        exit(1);
    }
    char buf[1024];
    while(1){
        size_t s = read(fd, buf, sizeof(buf)-1);
        if(s > 0){
            printf("client # %s\n",buf);
        }else{
            perror("read error");
            exit(2);
        }
    }
    close(fd);
    return 0;
}
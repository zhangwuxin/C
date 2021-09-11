#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <poll.h>

#define IPADDRESS "127.0.0.1"
#define PORT 6666
#define MAXSIZE 1024
#define LISTENQ 5
#define OPEN_MAX 10
#define INFTIM 1024

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    socklen_t serv_len = sizeof(serv_addr);
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    //初始化服务器
    memset(&serv_addr, 0, serv_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);
    //绑定IP和端口
    bind(lfd, (struct sockaddr *)&serv_addr, serv_len);
    //设置同时监听的最大个数
    listen(lfd, 36);
    printf("Start accept ......\n");
    int max = 1;
    int ret = 0;
    struct pollfd clientfds[OPEN_MAX];
    clientfds[0].fd = lfd;
    clientfds[0].events = POLLIN;
    int i;
    for (i = 1; i < OPEN_MAX; i++)
    {
        clientfds[i].fd = -1;
    }
    while (1)
    {
        ret = poll(clientfds, max + 1, INFTIM);
        if (ret < 0)
        {
            perror("poll error");
            exit(-1);
        }
        // 有客户端发起新连接
        if (clientfds[0].revents & POLLIN)
        {
            socklen_t clientlen = sizeof(sockaddr_in);
            int connfd = accept(lfd, (struct sockaddr *)(&serv_addr), &clientlen);
            if (connfd == -1)
            {
                perror("accept error");
                exit(1);
            }
            for (i = 1; i < OPEN_MAX; i++)
            {
                if (clientfds[i].fd < 0)
                {
                    clientfds[i].fd = connfd;
                    clientfds[i].events = POLLIN;
                    break;
                }
            }
            if (i == OPEN_MAX)
            {
                printf("to much clients\n");
                exit(0);
            }
            if (i > max - 1)
            {
                max = i + 1;
            }
            char ip[64];
            printf("new client IP: %s,Port: %d\n",
                   inet_ntop(AF_INET, &serv_addr.sin_addr.s_addr, ip, sizeof(ip)),
                   ntohs(serv_addr.sin_port));
        }
        //已经连接的客户端给我发送数据
        for (i = 1; i < OPEN_MAX; i++)
        {
            if (clientfds[i].fd == -1)
                continue;
            if (clientfds[i].revents & POLLIN)
            {
                char buf[1024] = {0};
                int readlen = read(clientfds[i].fd, buf, MAXSIZE);

                printf("recv buf: %s\n", buf);
                if (readlen == 0) //表明该fd已经关闭
                {
                    close(clientfds[i].fd);
                    clientfds[i].fd = -1;
                    continue;
                }
                // write(STDOUT_FILENO, buf, readlen);
                write(clientfds[i].fd, buf, readlen);
            }
        }
    }
}
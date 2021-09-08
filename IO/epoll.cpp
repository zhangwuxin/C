#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#define OPEN_MAX 100000

int main()
{
    struct epoll_event event;
    struct epoll_event *wait_event;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(10000);
    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(sockfd, 100);
    int sockNumber[OPEN_MAX] = {0};
    sockNumber[0] = sockfd;
    int crRet = epoll_create(10);
    if (-1 == crRet)
    {
        perror("创建文件描述符失败");
        return 0;
    }
    event.data.fd = sockfd;
    event.events = EPOLLIN;
    int clRet = epoll_ctl(crRet, EPOLL_CTL_ADD, sockfd, &event);
    if (-1 == clRet)
    {
        perror("注册监听事件类型失败");
    }
    int max1 = 0;
    char buf[1024] = {0};
    while (1)
    {
        wait_event = new epoll_event[max1 + 1];
        clRet = epoll_wait(crRet, wait_event, max1 + 1, -1);
        for (int i = 0; i < clRet; i++)
        {
            if ((sockfd == wait_event[i].data.fd) && (EPOLLIN == wait_event[i].events & EPOLLIN))
            {
                struct sockaddr_in cli_addr;
                socklen_t length = sizeof(cli_addr);
                sockNumber[max1 + 1] = accept(sockfd, (struct sockaddr *)&cli_addr, &length);
                if (sockNumber[max1 + 1] > 0)
                {
                    event.data.fd = sockNumber[max1 + 1];
                    event.events = EPOLLIN;
                    int ret1 = epoll_ctl(crRet, EPOLL_CTL_ADD, sockNumber[max1 + 1], &event);
                    max1++;
                    if (-1 == ret1)
                    {
                        perror("新连接的客户端注册失败");
                    }
                    printf("客户端%d上线\n", max1);
                }
            }
            else if (wait_event[i].data.fd > 3 && (EPOLLIN == wait_event[i].events & (EPOLLIN | EPOLLERR)))
            {
                memset(buf, 0, sizeof(buf));
                int len = recv(wait_event[i].data.fd, buf, sizeof(buf), 0);
                if (len <= 0)
                {
                    for (int j = 1; j <= max1; j++)
                    {
                        if (wait_event[i].data.fd == sockNumber[j])
                        {
                            clRet = epoll_ctl(crRet, EPOLL_CTL_DEL, wait_event[i].data.fd, wait_event + i);
                            printf("客户端%d下线\n", max1);
                            sockNumber[j] = sockNumber[max1];
                            close(sockNumber[max1]);
                            sockNumber[max1] = -1;
                            max1--;
                            usleep(50000);
                        }
                    }
                }
                else
                {
                    printf("%s\n", buf);
                }
            }
        }
        delete[] wait_event;
    }
    return 0;
}
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
#define PORT 7777
#define MAXSIZE 1024
#define LISTENQ 5
#define OPEN_MAX 10
#define INFTIM -1

int main()

{
    struct sockaddr_in my_addr;

    int serverfd = socket(AF_INET, SOCK_STREAM, 0);

    if (serverfd == -1)

    {

        perror("socket\n");

        return -1;
    }

    my_addr.sin_family = AF_INET;

    my_addr.sin_port = htons(PORT);

    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(serverfd, (struct sockaddr *)(&my_addr), sizeof(struct sockaddr));

    if (ret == -1)

    {

        perror("bind\n");

        return -2;
    }

    if (listen(serverfd, LISTENQ) == -1)

    {

        perror("listen\n");

        return -3;
    }

    printf("OK!\n");


    int max = 1;
    int i;
    ret = 0;

    if (serverfd < 0)

        return -1;

    struct pollfd clientfds[OPEN_MAX];

    clientfds[0].fd = serverfd;

    clientfds[0].events = POLLIN;

    for (i = 1; i < OPEN_MAX; i++)

    {

        clientfds[i].fd = -1;
    }

    while (1)

    {

        ret = poll(clientfds, max + 1, INFTIM);

        if (ret < 0)

        {

            perror("poll");

            exit(1);
        }

        if (clientfds[0].revents & POLLIN) //判断是否有连接

        {

            socklen_t clientlen = sizeof(sockaddr_in);

            int connfd = accept(serverfd, (struct sockaddr *)(&my_addr), &clientlen);

            if (connfd == -1)

            {

                perror("accept error!\n");

                exit(1);
            }
            else
            {
                for (i = 0; i < OPEN_MAX; i++)

                {

                    if (clientfds[i].fd = -1)

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

                    max = i + 1;
            }
        }

        for (i = 1; i < OPEN_MAX; i++)

        {

            if (clientfds[i].fd = -1)

                continue;

            if (clientfds[i].revents & (POLLRDNORM | POLLERR))

            {
                char buf[1024] = {0};

                int readlen = read(clientfds[i].fd, buf, MAXSIZE);

                if (readlen == 0) //表明该fd已经关闭

                {

                    close(clientfds[i].fd);

                    clientfds[i].fd = -1;

                    continue;
                }

                write(STDOUT_FILENO, buf, readlen);
                // send(i, buf, readlen, 0);
            }
        }
    }
}
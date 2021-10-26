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
#include <map>
#include <string>
#include <dlfcn.h>
#define OPEN_MAX 1024
#define PORT 33036
using namespace std;
int (*dlfun)(char *,char *);
//g++ epoll.cpp -o epoll -ldl

map<string, void *> ProcessSoMap;
int CallDllFun(const char *so, const char *command, char * fun, char * param, char *oparam)
{
	void *handle;
	int ret=1;
	char *dlError;

	string cmd(command);
	string soName = cmd + ".so";

	handle = ProcessSoMap[soName];
    

	if (handle ==  NULL){
        printf("handle null\n");
        if((handle = dlopen(so, RTLD_LAZY)) == NULL) {  
            printf("dlopen - %sn", dlerror());  
            exit(-1);  
        } 
	}else{
	}
   

	if (!handle) {
        printf("so name %s\n",so);
		strcpy(oparam, "result=-1001&msg=System Busy");
		return -1001;
	}
	dlfun =(int (*)(char *, char *)) dlsym(handle,fun);
	dlError = dlerror();
	if ( dlError)  {
        printf("dlopen - %sn", dlError);  
		strcpy(oparam, "result=-1002&msg=System Busy");
		dlclose(handle);
		ProcessSoMap[soName]= NULL;
		return -1002;
	}
	ret =(*dlfun)((char*)param,oparam);
	ProcessSoMap[soName] = handle;
	return ret;
}

int main()
{
    struct epoll_event event;
    struct epoll_event *wait_event;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
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
                char rsp[10000];
                memset(buf, 0, sizeof(buf));
                int len = recv(wait_event[i].data.fd, buf, sizeof(buf), 0);

                char so[100];
                sprintf(so, "/data/lainzhang/c_project/src/test.so",so);

                char so_name[100];
                sprintf(so_name, "test",so_name);

                char Request[100];
                sprintf(Request, buf,Request);
                int iRet=  CallDllFun(so, so_name, "MyProcess", Request, rsp);
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
                    write(wait_event[i].data.fd, rsp, strlen(rsp));
                }
            }
        }
        delete[] wait_event;
    }
    close(sockfd);
    return 0;
}
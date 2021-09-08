#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
       
int main(int argc,char *argv[])
{          
    struct sockaddr_in serv_addr;
    socklen_t serv_len = sizeof(serv_addr);
    int lfd = socket(AF_INET,SOCK_STREAM,0);
    //初始化服务器
    memset(&serv_addr,0,serv_len);
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(6767);
    //绑定IP和端口
    bind(lfd,(struct sockaddr*)&serv_addr,serv_len);
    //设置同时监听的最大个数
    listen(lfd,36);
    printf("Start accept ......\n");
       
    struct sockaddr_in client_addr;
    socklen_t cli_len = sizeof(client_addr);
    // 最大的文件描述符
    int maxfd = lfd;
    // 文件描述符读集合
    fd_set reads,temp; 
    // init    
    FD_ZERO(&reads);
    FD_SET(lfd,&reads);
    while(1)
    {    
       //委托内核IO检测                                                                                  
       temp = reads;
       int  ret = select(maxfd+1,&temp,NULL,NULL,NULL);
       if(ret == -1) 
       {   
            perror("select error");
            break;
       } else if(ret == 0){
          continue;
      }
       // 有客户端发起新连接
       if (FD_ISSET(lfd,&temp)) {
           //接收连接请求 accept不阻塞
           int cfd = accept(lfd,(struct sockaddr*)&client_addr,&cli_len);
           if(cfd == -1) 
           {   
               perror("accept error");
               exit(1);
           }   
           char ip[64];
           printf("new client IP: %s,Port: %d\n",
                  inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,ip,sizeof(ip)),
                  ntohs(client_addr.sin_port));
           // 将cfd加入到带检测的读集合中 下一次就可以检测到了
           FD_SET(cfd,&reads);
           //更新最大的文件描述符
           maxfd = maxfd < cfd?cfd:maxfd;
        // 如果只有lfd变化
          if(--ret == 0)
                continue;
       }   
       //已经连接的客户端给我发送数据
       for (int i =lfd+1; i <= maxfd; ++i) {                                                                                               
           if(FD_ISSET(i,&temp))
           {   
               char buf[1024] = {0};
               int len = recv(i,buf,sizeof(buf),0);
               if(len < 0) 
               {   
                   perror("recv error");
                   close(i);
                    FD_CLR(i,&reads);
               }else if(len == 0)
               {   
                   printf("客户端已经断开连接\n");
                   close(i);
                   //从读集合中删除
                   FD_CLR(i,&reads);
               }else{
                    printf("recv buf: %s\n",buf);
                    send(i,buf,len,0);
               }   
           }   
       }   
    }   
    close(lfd);
    return 0;
}
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

struct msg
{
    /* data */
    long msgtype;
    char msgdata[50];
};

//msgrcv 读取消息
int main()
{
    int msgid;
    msgid = msgget(16, IPC_CREAT|IPC_EXCL|0777);
    if(msgid == -1){
        if (errno == EEXIST){
            msgid = msgget(16, 0777);
        }else{
            perror("create failed\n");
            return -1;
        }
    }

    printf("msgid : %d\n", msgid);
    struct msg msg1;
    bzero(&msg1, sizeof(msg1));
    msg1.msgtype = 520;
    msgrcv(msgid, &msg1, sizeof(msg1), msg1.msgtype, 0);
    printf("receive msg : %s \n",msg1.msgdata);
    return 0;
}
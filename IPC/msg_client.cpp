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


int main()
{
    int msgid;
    msgid = msgget(16, IPC_CREAT|IPC_EXCL|0777);
    if(msgid == -1){
        printf("1\n");
        if (errno == EEXIST){
            printf("2\n");
            msgid = msgget(16, 0777);
        }else{
            printf("3\n");
            perror("create failed\n");
            return -1;
        }
    }

    printf("msgid : %d\n", msgid);
    struct msg msg1;
    bzero(&msg1, sizeof(msg1));
    printf("4\n");
    msg1.msgtype = 520;
    // strcpy(msg1.msgdata, "hello world!");
    msgrcv(msgid, &msg1, sizeof(msg1), 520, 0);
    printf("receive msg : %s \n",msg1.msgdata);
    // msgsnd(msgid, &msg1, sizeof(msg1), 0);
    return 0;
}
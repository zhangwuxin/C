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

//int msgget(key_t key, int msgflag)，用于创建一个新的或打开一个已经存在的消息队列，此消息队列与key相对应。
//msgsnd 将消息写入队列
// 1.消息队列是消息的链表,具有特定的格式,存放在内存中并由消息队列标识符标识.
// 2.消息队列允许一个或多个进程向它写入与读取消息.
// 3.管道和命名管道都是通信数据都是先进先出的原则。
// 4.消息队列可以实现消息的随机查询,消息不一定要以先进先出的次序读取,也可以按消息的类型读取.比FIFO更有优势。
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
    struct msg msg1;
    bzero(&msg1, sizeof(msg1));
    msg1.msgtype = 520;
    strcpy(msg1.msgdata, "hello world!");
    msgsnd(msgid, &msg1, sizeof(msg1), 0);
    return 0;
}
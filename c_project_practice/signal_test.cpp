#include <iostream>
#include <csignal>
#include <unistd.h>
#include <cstdlib>
using namespace std;

void signalhandler( int signum){
    cout << "Interrupt signal : " << signum << " received " << endl;
    exit(signum);
}
int main(int argc, char ** argv) {
    int c;
    //signal(SIGINT, signalhandler);
    //signal(SIGINT,SIG_IGN);//忽略InterruptKey
   //signal(SIGHUP,SIG_IGN);//终端关闭时，该信号被发送到session首进程及作为job提交的进程(即&提交的进程)
   signal(SIGQUIT,SIG_IGN);
   //和SIGINT类似, 但由QUIT字符(通常是Ctrl-\)来控制. 进程在因收到SIGQUIT退出时会产生core文件, 在这个意义上类似于一个程序错误信号。
    c = getopt(argc, argv, "sp");
    switch(c)
    {
        case 's':
            cout << "s" << endl;
            break;
        case 'p':
            cout << "p" << endl;
            break;
        case '?':
            cout << "unknow" << endl;
            return 0;
    }
    while (1){
        cout << "going to sleep ..." << endl;
        sleep(1);
    }
    return 0;
}


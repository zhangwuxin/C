#include <iostream>
#include <csignal>
#include <unistd.h>
#include <cstdlib>
using namespace std;

inline void signalhandler( int signum){
    cout << "Interrupt signal : " << signum << " received " << endl;
    exit(signum);
}
void p_out()
{
    cout << "test !! " << endl;
}
int main(int argc, char ** argv) {
    int c;
    signal(SIGINT, signalhandler);
//    while (1){
//    sleep(1);
//    }
    int a = rand();
    cout << a << endl;
    p_out();
    char ch  ;
    cin >> ch;
    int i=ch;
    cout << ch << " " << i << endl;
    ch += 1;
    i = ch;
    cout << ch << " " << i << endl;
    if (1 == 1)
    {
        string cc = "1";
        
    }
    
    string cc = "1";
    
    return 0;
}


#include <iostream>
using namespace std;

typedef int (*CallBack)(char *p);

int A(char *p)
{
    cout << "A" << endl;
    cout << p << endl;
    return 0;
}

int B(CallBack lpCall, char *p)
{
    cout << "B" << endl;
    cout << p << endl;
    lpCall(p);
    return 0;
}

int main()
{
    char *p = "hello";
    B(A,p);
    return 0;
}

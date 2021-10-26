#include <string.h>
#include <stdio.h>
using namespace std;

// g++ -fPIC -shared -o test.so -cpp test.cpp

extern "C"  int MyProcess(char *in_param, char *out_param);
int MyProcess(char *in_param, char *out_param)
{
    int res=0;
    strcpy(out_param,in_param);
    return res;
}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
using namespace std;

int a[10000][10000];
#define get_cost_time(he) TIME(he)
//先访问行
void fun_1()
{
    int i, j;
    
    for (i = 0; i < 10000; i++)
    {
        for (j = 0; j < 10000; j++)
        {
            a[i][j] = i;
        }
    }
    cout << __FUNCTION__ << ": ";
}


//先访问行
void fun_2()
{
    int i, j;

    for (i = 0; i < 10000; i++)
    {
        for (j = 0; j < 10000; j++)
        {
            int _ = a[i][j];
        }
    }
    cout << __FUNCTION__ << ": ";
}

//先访问列
void fun_3()
{
    int i, j;

    for (j = 0; j < 10000; j++)
    {
        for (i = 0; i < 10000; i++)
        {
            int _ = a[i][j];
        }
    }
    cout << __FUNCTION__ << ": ";
}

double get_cost_time(void (*pf)())
{
    clock_t start, finish;

    double duration;

    start = clock();

    (*pf)();

    finish = clock();

    duration = (double)(finish - start) / CLOCKS_PER_SEC;

    cout << duration << " seconds" << endl;
}

int main()
{

    // clock_t start, finish;

    // double duration;

    // start = clock();

    // fun_1();

    // finish = clock();

    // duration = (double)(finish - start) / CLOCKS_PER_SEC;

    // cout << "func_1 : " << duration << " seconds" << endl;

    // start = clock();

    // fun_2();

    // finish = clock();

    // duration = (double)(finish - start) / CLOCKS_PER_SEC;

    // cout << "func_2 : " << duration << " seconds" << endl;

    // start = clock();

    // fun_3();

    // finish = clock();

    // duration = (double)(finish - start) / CLOCKS_PER_SEC;

    // cout << "func_3 : " << duration << " seconds" << endl;

    TIME(fun_1);
    TIME(fun_2);
    TIME(fun_3);

    return 0;
}
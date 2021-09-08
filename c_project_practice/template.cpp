#include<iostream>


using namespace std;


template <typename T>
void Swap (T& a, T& b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

int main()
{
    int a = 1;
    int b = 2;
    cout << a << " " << b << endl;
    Swap(a, b);
    cout << a << " " << b << endl;
    float c = 1.1;
    float d = 2.1;
    
    cout << c << " " << d << endl;
    Swap(c, d);
    cout << c << " " << d << endl;
    return 0;
}


#include <unistd.h>
#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
    const char* filename = "a.txt";
    if ((access(filename, 0)) != -1){
        cout << "File Exit" << endl;
        if ((access(filename, 2)) != -1)
        cout << "File Can wirte" << endl;
    }else{
        cout << "File Not Exit" << endl;
    }
    return 0;
}
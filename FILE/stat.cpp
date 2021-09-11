#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
    struct stat buf;
    const char* filename = "a.txt";
    int result = stat(filename, &buf);
    cout << "FileSize : " << buf.st_size << endl;
    cout << "LastTime : " << buf.st_atime << endl;
    return 0;
}
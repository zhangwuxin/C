#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char **argv)
{
    string content = "hello world";
    ofstream file;
    string filename = "/data/lainzhang/files/test.txt";
    file.open("/data/lainzhang/files/test.txt", ios::app);
    if (!file.is_open()){
        cout << "failed to open file!" << filename << endl;
        return -1;
    }
    file << content << endl;
    file.close();
    return 0;
}
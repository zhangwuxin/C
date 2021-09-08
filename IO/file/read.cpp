#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char **argv)
{
    ifstream file;
    string filename = "/data/lainzhang/files/test.txt";
    file.open("/data/lainzhang/files/test.txt");
    if (!file.is_open()){
        cout << "failed to open file!" << filename << endl;
        return -1;
    }
    stringstream buffer;  
    buffer << file.rdbuf();  
    file.close();
    cout << "Read file content is \n" << buffer.str() << endl;
    return 0;
}
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/types.h>   
#include <sys/stat.h>
using namespace std;

int main(int argc, char **argv)
{
    chdir("/data/lainzhang/c_project/src");
    cout << "Current Path " << getcwd(NULL, NULL) << endl;
    const char* filename = "a.txt";
    chmod(filename, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    ofstream file;
    file.open(filename, ios::app);
    string content = "append content";
    file << content ;

    ifstream file_;
    file_.open(filename);
    if (!file_.is_open()){
        cout << "failed to open file!" << filename << endl;
        return -1;
    }
    stringstream buffer;  
    buffer << file_.rdbuf();  
    file_.close();
    cout << "Read file content is \n" << buffer.str() << endl;
}
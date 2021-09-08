#include <vector>
#include <iostream>
#include <string.h>

using namespace std;

int main()
{
    vector <string> str_list;
    str_list.push_back("A");
    cout << &str_list[0] << endl;
    str_list.push_back("A");
    cout << &str_list[1] << endl;
    return 0;
}

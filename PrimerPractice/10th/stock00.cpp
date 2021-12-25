#include <iostream>
#include "stock00.h"
using namespace std;

Stock::Stock(){
    company = "no name";
}
void Stock::acquire(const std::string & co, long n, double pr)
{
    company = co;
    if (n < 0)
    {
        std::cout << "Number of shares can't be negative;"
            << company << "shares set to 0.\n";
        shares = 0;
    }
    else
        shares = n;
    share_value = pr;
    set_tot();
}
Stock::~Stock()
{
    
}

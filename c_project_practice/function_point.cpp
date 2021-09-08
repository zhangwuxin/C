#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

double cal_m1(int lines)
{
    cout << __FUNCTION__ << " -> ";
    return 0.1 * lines;
}


double cal_m2(int lines)
{
    cout << __FUNCTION__ << " -> " ;
    return 0.01 * lines;
}

void estimate(int line_num, double (*p)(int lines))
{
    cout << "the " << line_num << " need time is : " << (*p)(line_num) << endl;
}


int main()
{
    int line_num = 10;
    double (*func_list[]) (int) = {cal_m1, cal_m2};
    enum f_type {
        one_point,
        two_point,
    };
    estimate(line_num, func_list[one_point]);
    estimate(line_num, func_list[two_point]);
    return 0;
}

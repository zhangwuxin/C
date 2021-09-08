//
// Created by lainzhang on 2019/12/3.
//


#include "../include/DataAbstract.h"
using namespace DataAbs;

int outTotal(){
    Adder a;
    a.addNum(1);
    a.addNum(2);
    cout << "Total : " << a.getTotal() << endl;
    return 0;
}
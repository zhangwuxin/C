//
// Created by lainzhang on 2019/11/30.
//

#include "../include/StdOut.h"
#include <iostream>
#include <string>

using namespace StdOut;
int StdOutFun() {
    StdOut::StdOutClass Stout;
    Stout.name = "lainzhang";
    Stout.age  = 25;
//    cin >> Stout.name >> Stout.age;
    cout << Stout.name << " is "<<Stout.age << " years old!" << endl;
    return 0;
};

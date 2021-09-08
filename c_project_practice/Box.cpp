//
// Created by lainzhang on 2019/12/2.
// 重载运算符和重载函数
//

#include "../include/Box.h"
#include <iostream>



int OutBox(){
    Box Box1;
    Box Box2;
    Box Box3;
    double volume = 0.0;
    Box1.setLBHth(1,2,3);
    Box2.setLBHth(1,2,3);
    volume = Box1.getVolume();
    cout << "Volume of Box1 : " << volume << endl;
    volume = Box2.getVolume();
    cout << "Volume of Box2 : " << volume << endl;
    Box3 = Box1 + Box2;
    volume = Box3.getVolume();
    cout << "Volume of Box3 : " << volume << endl;
    return 0;
}
int OutData(){
    Data pd;
    pd.print(1);
    pd.print("hello world");
    pd.print(2.222222);
    return 0;
}
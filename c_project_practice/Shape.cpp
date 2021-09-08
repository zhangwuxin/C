//
// Created by lainzhang on 2019/12/2.
//继承
//
#include <iostream>
#include "../include/Shape.h"

int OutShape(){
    Rectangle Rect;
    Rect.setHeight(5);
    Rect.setWidth(10);
    cout << "Area : " << Rect.getArea() << endl;
    return 0;
}

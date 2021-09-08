//
// Created by lainzhang on 2019/12/2.
//多态
//

#include "../include/DyShape.h"
using namespace DyShapeNamespace;

int DyShapeOut(){
    DyShape *shape;
    Rectangle rec(10,7);
    Triangle tri(10,5);
    shape = &rec;
    cout << shape->area() << endl;
    shape = &tri;
    cout << shape->area() << endl;
    return 0;
}
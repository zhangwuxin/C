//
// Created by lainzhang on 2019/12/2.
//

#include "../include/ClassObject.h"
using namespace ClassObjectNamespaceWithoutKey;

//不带参数构造函数
Line::Line(){
    cout << "object is being created" << endl;
}

void Line::setLength(double len) {
    length = len;
}
double Line::getLength() {
    return length;
}

//带参数构造函数
Line2::Line2(double len):length(len) {
    cout << "Object is being created,length=" << len << endl;
//    length = len;
}
void Line2::setLength(double len) {
    length = len;
}
double Line2::getLength(void) {
    return length;
}

//类的析构函数
Line3::Line3(void) {
    cout << "Object is being created" << endl;
}
Line3::~Line3() {
    cout << "Object is being deleted" << endl;
}
void Line3::setLength(double len) {
    length = len;
}
double  Line3::getLength() {
    return length;
}
int ClassObjectOut(){
    Line line;
    // 设置长度
    line.setLength(6.0);
    cout << "Length of line : " << line.getLength() <<endl;
    Line2 line2(10.0);
    cout << "Length of line :" << line2.getLength() << endl;
    line2.setLength(2.0);
    cout << "Length of line :" << line2.getLength() << endl;
    Line3 line3;
    line3.setLength(9.0);
    cout << "Length of line : " << line3.getLength() << endl;
    return 0;
}

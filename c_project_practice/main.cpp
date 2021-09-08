#include <iostream>
#include <csignal>
#include <unistd.h>
#include "../include/NameSayOut.h"
#include "../include/StdOut.h"
#include "../include/Shape.h"
#include "../include/Box.h"
#include "../include/DyShape.h"
#include "../include/ClassObject.h"
#include "../include/DataAbstract.h"
#include "../include/fileDeal.h"
#include "../include/ DyCache.h"
#include "../include/templateObj.h"
#include "../include/StringObj.h"
#include "../include/LuaDealTest.h"

void signalhandler( int signum){
    cout << "Interrupt signal : " << signum << " received " << endl;
    exit(signum);
}
int main() {
//    cout << "Hello, World!" << endl;
//    NameSayOut();
//    StdOutFun();
//    OutShape(); //继承
//    OutBox();   //重载
//    OutData();
//    DyShapeOut();  //多态
//    ClassObjectOut(); //构造函数
//    outTotal();  //数据抽象
//    fileDeal file; //文件和流
//    file.outFile();
//    OutCache(); //动态内存
//    templateOut();
//    StringObj strobj;
//    strobj.StrOut();
//    LuaDealTest lua;
//    string luastring = "o_g_time=1&o_type=2&<o_g_time2=\"echo>2\"&>o_g_name=lain";
//    lua.find_global_put(luastring);
    printf("111");
    signal(SIGINT, signalhandler);
    while (1){
        cout << "going to sleep ..." << endl;
        sleep(1);
    }
    return 0;
}


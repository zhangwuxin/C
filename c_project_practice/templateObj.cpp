//
// Created by lainzhang on 2019/12/6.
//

#include "../include/templateObj.h"

int templateOut(){
    try{
        Stack<int> intStack;
        Stack<string> stringStack;
        intStack.push(7);
        cout << intStack.top() << std::endl;
        stringStack.push("LLksdjfksdj");
        cout << stringStack.top() << std::endl;
//        stringStack.empty();
        stringStack.pop();
        stringStack.pop();
    }
    catch(exception const& ex) {
//        cout << "Exception : " << ex.what() << endl;
        cerr << "Exception2 : " << ex.what() << endl;
        return -1;
    }
    return 0;
}


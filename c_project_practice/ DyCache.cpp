//
// Created by lainzhang on 2019/12/5.
//

#include "../include/ DyCache.h"


int OutCache(){
    double* pvalue = NULL;
    pvalue = new double;
    *pvalue = 23;
    cout << "Value of pvalue :" << *pvalue << endl;
    delete pvalue;
    cout << "Value of pvalue :" << *pvalue << endl;
    return 0;
}
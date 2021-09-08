//
// Created by lainzhang on 2020/6/11.
//

#include "../include/node_list.h"
using namespace std;

typedef struct node{
    int data;
    struct node* next;
}NODE;

class Linklist{
public:
    Linklist(){head = nullptr;}
    ~Linklist();
    bool clearSqList();
    bool isEmpty(){return head == nullptr;};
    int Length();
    bool GetElem(int i, int* e);
    int LocateElem(int e);
    bool PriorElem(int cur_e, int* pre_e);
    bool  NextElem(int cur_e, int* next_e);
    bool Insert(int i, int e);
    bool Delete(int i, int *e);
    NODE* Reverse();

private:
    NODE* head;
};

//清空函数
bool Linklist::clearSqList() {
    NODE *p = head;
    while(head){
        p = head;
        head = head->next;
        delete p;
    }
}

//析构函数
Linklist::~Linklist() {
    NODE *p = head;
    while (head){
        p = head;
        head = head->next;
        delete p;
    }
}

//获取链表长度
int Linklist::Length() {
    NODE *p = head;
    int len = 0;
    while (p != nullptr){
        len++;
        p = p->next;
    }
    return len;
}
#include <string>
#include <iostream>
using namespace std;

struct Node
{
    int data;
    Node *next;
};

struct List
{
    Node *head;
    int length;
};

void AddNode(List &list, int dt)
{
    Node *pCurNode = new Node; //开辟一个Node类型数据的空间，将地址返回给pCurNode
    pCurNode->data = dt;
    pCurNode->next = NULL;
    if (list.head == NULL) //如果当前插入的是链表第一个节点
    {
        list.head = pCurNode;
        list.length = 1;
    }
    else //尾插法
    {
        Node *pt = list.head;
        while (pt->next != NULL)
            pt = pt->next;
        pt->next = pCurNode;
        list.length++;
    }
}

void DeleteNode(List &list, int num)
{
    Node *pCurNode = list.head;
    Node *preNode = NULL;
    while (pCurNode != NULL && pCurNode->data != num)
    {
        preNode = pCurNode;
        pCurNode = pCurNode->next; // 节点后移
    }
    if (pCurNode == NULL)
    {
        cout << "Can't find " << num << "in the list" << endl;
        return;
    }
    if (pCurNode == list.head)
        list.head = list.head->next;
    else
        preNode->next = pCurNode->next;
    list.length--;
    delete pCurNode;
}

Node *FindNode(const List &list, int num)
{
    Node *pCurNode = list.head;
    while (pCurNode)
    {
        if (pCurNode->data == num)
        {
            cout << "find it " << num << "\n";
            return pCurNode;
        }
        pCurNode = pCurNode->next;
    }
    cout << "cant'not fund it" << endl;
    return NULL;
}

void Output(const List &list)
{
    cout << "List : " << endl;
    Node *pCurNode = list.head;
    while (pCurNode != NULL)
    {
        cout << pCurNode->data;
        if (pCurNode->next)
            cout << "->";
        pCurNode = pCurNode->next;
    }
    cout << endl;
}

void ReverseList(Node *head)
{
    cout << "head->data : " << head->data << endl;
    if (head == NULL || head->next== NULL)
        cout <<"list not need reverse" << endl;
    else{
        Node *t = NULL;
        Node *p = head;
        Node *q = head->next;
        while(q!= NULL)
        {
            t = q->next;
            q->next = p;
            p = q;
            q = t;
        }
        cout << "p->data " << p->data << endl;
        cout << "p->next->data " << p->next->data << endl;
        try
        {

            cout << "head->next->data" << head->next->data << endl;
            cout << "head->data" << head->data << endl;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
        head->next=NULL;
        head = p;
        Node *t2 = head;
        while (t2->next)
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
            t2=t2->next;
        }
        t2->next=NULL;
        cout << "head->next->data" << head->next->data << endl;
        cout << "head->data" << head->data << endl;
        
    }
}

int main()
{
    List lst;
    lst.head = NULL;
    lst.length = 0;
    int max;
    cin >> max;
    for (int i = 1; i <= max; i++)
    {
        AddNode(lst, i);
    }
    Output(lst);
    Node *c = FindNode(lst, 2);
    cout << c->data << endl;
    // DeleteNode(lst, 3);
    Output(lst);

    ReverseList(lst.head);
    Output(lst);

    return 0;
}
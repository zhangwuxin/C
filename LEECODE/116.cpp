#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
#include <algorithm>
using namespace std;


// Definition for a Node.
class Node {
public:
    int val;
    Node* left;
    Node* right;
    Node* next;

    Node() : val(0), left(NULL), right(NULL), next(NULL) {}

    Node(int _val) : val(_val), left(NULL), right(NULL), next(NULL) {}

    Node(int _val, Node* _left, Node* _right, Node* _next)
        : val(_val), left(_left), right(_right), next(_next) {}
};

class Solution {
public:
    // Node* connect(Node* root) {
    //     if (root == nullptr)
    //     {
    //         return root;
    //     }
    //     queue<Node*> Q;
    //     Q.push(root);
    //     while (!Q.empty())
    //     {
    //         /* code */
    //         int size = Q.size();
    //         for (int i=0; i < size; i++)
    //         {
    //             Node* node = Q.front();
    //             Q.pop();
    //             if(i < size -1)
    //             {
    //                 node->next = Q.front();
    //             }
    //             if(node->left != nullptr)
    //             {
    //                 Q.push(node->left);
    //             }
    //             if(node->right != nullptr)
    //             {
    //                 Q.push(node->right);
    //             }
    //         }
    //     }
    //     return root;
    // }

    Node* connect(Node* root) {
        if (root == nullptr)
        {
            return root;
        }
        Node *leftNode = root;
        while(leftNode->left!=nullptr)
        {
            Node *head = leftNode;
            while (head!=nullptr)
            {
                /* code */
                head->left->next = head->right;
                if(head->next!=nullptr)
                {
                    head->right->next = head->next->left;
                }
                head = head->next;
            }
            leftNode = leftNode->left;
        }
        return root;
    }
};


int main(){
    Solution search_obj;
    int num = search_obj.connect(512);
    cout << num << endl;
}

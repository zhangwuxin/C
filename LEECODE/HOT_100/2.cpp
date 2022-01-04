#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
#include <unordered_map>
using namespace std;


struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

class Solution {
public:
    ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
        ListNode *head = nullptr, *tail = nullptr;
        int carry = 0;
        while (l1 || l2)
        {
            /* code */
            int n1 = l1 ? l1->val:0;
            int n2 = l2 ? l2->val:0;
            int sum = n1 + n2 + carry;
            if (!head){
                head = tail = new ListNode(sum % 10);
            } else {
                tail->next = new ListNode(sum % 10);
                tail = tail->next;
            }
            carry = sum / 10;
            if(l1){
                l1 = l1->next;
            }
            if(l2)
            {
                l2 = l2->next;
            }
        }
        if (carry > 0)
        {
            tail->next = new ListNode(carry);
        }
        return head;
    }
};

int main(){
    Solution search_obj;
    vector<int> nums = {1, 2, 3, 5, 7};
    vector<int> new_v = search_obj.twoSum(nums,8);
    for(int i=0;i<new_v.size();i++)
        cout << new_v[i] << endl;
}

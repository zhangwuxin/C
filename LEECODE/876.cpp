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
    ListNode* middleNode(ListNode* head) {
        ListNode *slow = head;
        ListNode *fast = head;
        while(fast != NULL && fast->next != NULL)
        {
            slow = slow->next;
            fast = fast->next->next;
        }
        return slow;

    }
};

int main(){
    Solution search_obj;
    vector<int> nums = {1, 2, 3, 5, 7};
    vector<int> new_v = search_obj.twoSum(nums,8);
    for(int i=0;i<new_v.size();i++)
        cout << new_v[i] << endl;
}

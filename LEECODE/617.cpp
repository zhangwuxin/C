#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
#include <algorithm>
using namespace std;



// Definition for a binary tree node.
struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

class Solution {
public:
    TreeNode* mergeTrees(TreeNode* root1, TreeNode* root2) {
        if(root1 == nullptr)
        {
            return root2;
        }
        if (root2 == nullptr)
        {
            return root1;
        }
        auto mergerd = new TreeNode(root1->val + root2->val);
        mergerd->left = mergeTrees(root1->left,root2->left);
        mergerd->right = mergeTrees(root1->right,root2->right);
        return mergerd;
    }
};
int main(){
    Solution search_obj;
    int num = search_obj.mergeTrees(15);
    cout << num << endl;
}

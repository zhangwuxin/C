#include <string.h>
#include <vector>
#include <iostream>
#include <string>
using namespace std;

class Solution {
public:
    void moveZeroes(vector<int>& nums) {
        int left = 0, right = 0, n = nums.size();
        while (right < n)
        {
            /* code */
            if (nums[right]){
                swap(nums[left], nums[right]);
                left++;
            }
            right++;
        }
        
    }
};

int main(){
    Solution search_obj;
    vector<int> nums = {1,0,2,3,0,4};
    search_obj.moveZeroes(nums);
    for(int i=0;i<nums.size();i++)
        cout << nums[i] << endl;
}
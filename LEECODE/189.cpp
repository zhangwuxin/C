#include <string.h>
#include <vector>
#include <iostream>
#include <string>
using namespace std;

class Solution {
public:
    // void rotate(vector<int>& nums, int k) {
    //     vector <int> new_vec (nums.size());
    //     for(int i=0;i<nums.size();i++)
    //     {
    //         new_vec[(i+k)%(nums.size())] = nums[i];
    //     }
    //     nums = new_vec;
    // }

    // void reverse(vector<int>& nums, int start, int end)
    // {
    //     while (start < end)
    //     {
    //         swap(nums[start],nums[end]);
    //         start++;
    //         end--;
    //     }
    // }
    // void rotate(vector<int>& nums, int k) {
    //     k %= nums.size();
    //     reverse(nums, 0, nums.size()-1);
    //     reverse(nums, 0, k-1);
    //     reverse(nums, k, nums.size()-1);
    // }

    void rotate(vector<int>& nums, int k) {
        
    }
};

int main(){
    Solution search_obj;
    vector<int> nums = {1,2,3,4,5,6,7};
    search_obj.rotate(nums,3);
    for(int i=0;i<nums.size();i++)
        cout << nums[i] << endl;
}
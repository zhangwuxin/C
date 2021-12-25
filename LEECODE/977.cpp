#include <string.h>
#include <vector>
#include <iostream>
#include <string>
using namespace std;

class Solution {
public:
    vector<int> sortedSquares(vector<int>& nums) {
        vector<int> result_vec (nums.size());
        int pos = nums.size() - 1, left = 0,right = nums.size() - 1;
        while (left <= right){
            if(nums[left]*nums[left] < nums[right]*nums[right])
            {
                result_vec[pos] = nums[right]*nums[right];
                right--;
            }else{
                result_vec[pos] = nums[left]*nums[left];
                left++;
            }
            pos--;
        }
        return result_vec;
    }
};

int main(){
    Solution search_obj;
    vector<int> nums = {-4,-1,0,3,10};
    vector<int> new_vec = search_obj.sortedSquares(nums);
    for(int i=0;i<new_vec.size();i++)
        cout << new_vec[i] << endl;
}
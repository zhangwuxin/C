#include <string.h>
#include <vector>
#include <iostream>
#include <string>
using namespace std;

class Solution {
public:
    vector<int> twoSum(vector<int>& numbers, int target) {
        int left = 0, right = numbers.size() - 1;
        while(left < right)
        {
            if(numbers[left] + numbers[right] < target)
            {
                left++;
            }else if (numbers[left] + numbers[right] > target){
                right--;
            }else{
                return {left+1,right+1};
            }
        }
        return {-1, -1};
    }
};

int main(){
    Solution search_obj;
    vector<int> nums = {0,1,3,5,7,9};
    nums = search_obj.twoSum(nums,4);
    for(int i=0;i<nums.size();i++)
        cout << nums[i] << endl;
}
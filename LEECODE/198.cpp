#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;


class Solution {
public:
    int rob(vector<int>& nums) {
        if(nums.size() < 0)
        {
            return 0;
        }
        else if (nums.size() == 1)
        {
            return nums[0];
        }else
        {
            vector<int> dp = vector<int> (nums.size());
            dp[0] = nums[0];
            dp[1] = max(nums[0], nums[1]);
            for (int i = 2; i < nums.size() ; i++)
            {
                dp[i] = max(dp[i-2] + nums[i], dp[i-1]);
            }
            return dp[nums.size() - 1];
        }
    }
};

int main(){
    Solution search_obj;
    vector<int> nums = {1,0,2,3,0,4};
    int num = search_obj.rob(nums);
    cout << num << endl;
}

#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
#include <unordered_map>
using namespace std;


class Solution {
public:
    // vector<int> twoSum(vector<int>& nums, int target) {
    //     for(int i=0; i<nums.size(); i++)
    //     {
    //         for (int j=i+1; j<nums.size(); j++)
    //         {
    //             if (target == nums[i]+nums[j]){
    //                 return {i, j};
    //             }
    //         }
    //     }
    //     return {};
    // }

    vector<int> twoSum(vector<int>& nums, int target) {
        unordered_map <int, int> hash_table;
        for(int i=0; i<nums.size(); i++)
        {
            auto it = hash_table.find(target - nums[i]);
            if(it != hash_table.end()){
                return {it->second, i};
            }
            hash_table[nums[i]] = i;
        }
        return {};
    }

};

int main(){
    Solution search_obj;
    vector<int> nums = {1, 2, 3, 5, 7};
    vector<int> new_v = search_obj.twoSum(nums,8);
    for(int i=0;i<new_v.size();i++)
        cout << new_v[i] << endl;
}

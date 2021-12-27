#include <string.h>
#include <vector>
#include <iostream>
#include <string>
using namespace std;

class Solution {
public:
    void reverseString(vector<char>& s) {
        int left = 0, right = s.size() - 1;
        while (left < right)
        {
            /* code */
            swap(s[left], s[right]);
            left++;
            right--;
        }
        
    }
};

int main(){
    Solution search_obj;
    vector<char> nums = {'h','e','l'};
    search_obj.reverseString(nums);
    for(int i=0;i<nums.size();i++)
        cout << nums[i] << endl;
}
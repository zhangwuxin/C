#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;


class Solution {
public:
    int singleNumber(vector<int>& nums) {
        int ret = 0;
        for (auto e: nums) ret ^= e;
        return ret;
    }
};
int main(){
    Solution search_obj;
    vector<int> x = {1,2,3,3,1};
    int num = search_obj.singleNumber(x);
    cout << num << endl;
}

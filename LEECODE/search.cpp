#include <string.h>
#include <vector>
#include <iostream>
using namespace std;


class Solution {
public:
    int search(vector<int>& nums, int target) {
        int low = 0;
        int high = nums.size()-1;
        while(low <= high){
            int middle = (high - low )/2 + low;
            int num = nums[middle];
            if (num < target){
                low = middle + 1;
            }else if (num > target){
                high = middle - 1;
            }else{
                return middle;
            }
        }
        return -1;
    }
};
int main(){
    int a[5]={1,2,3,4,5};
    vector<int> vec_i(a,a+5);
    Solution search_obj;
    int num = search_obj.search(vec_i, 3);
    cout << num << endl;
    return 0;
}

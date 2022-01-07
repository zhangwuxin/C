#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
#include <algorithm>
using namespace std;


class Solution {
public:
    int hammingWeight(uint32_t n) {
        int ret=0;
        while(n)
        {
            n &= n-1;
            ret++;
        }
        return ret;
    }
};
int main(){
    Solution search_obj;
    int num = search_obj.hammingWeight(15);
    cout << num << endl;
}

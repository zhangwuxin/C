#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;


class Solution {
public:
    int climbStairs(int n) {
        int p = 0, q = 0, r = 1;
        for(int i=0; i<n; i++)
        {
            p = q;
            q = r;
            r = p + q;
        }
        return r;
    }
};

int main(){
    Solution search_obj;
    int num = search_obj.climbStairs(5);
    cout << num << endl;
}

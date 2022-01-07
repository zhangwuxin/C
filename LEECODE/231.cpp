#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
#include <algorithm>
using namespace std;


class Solution {
public:
    bool isPowerOfTwo(int n) {
        return n > 0 && (n & (n - 1)) == 0;
    }
};

int main(){
    Solution search_obj;
    int num = search_obj.isPowerOfTwo(512);
    cout << num << endl;
}

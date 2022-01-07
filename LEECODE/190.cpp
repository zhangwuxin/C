#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;


class Solution {
public:
    uint32_t reverseBits(uint32_t n) {
        uint32_t rec = 0;
        for(int i=0; i<32 && n>0; i++)
        {
            rec |= (n & 1) << (31-i);
            n >>= 1;
        }
        return rec;
    }
};

int main(){
    Solution search_obj;
    uint32_t a = 00000010100101000001111010011100;
    uint32_t num = search_obj.reverseBits(a);
    cout << num << endl;
}

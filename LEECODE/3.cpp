#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;


class Solution {
public:
    int lengthOfLongestSubstring(string s) {
        unordered_set<char> occ;
        int n = s.size();
        int rk = - 1, ans = 0;
        for (int i = 0; i < n; i++){
            if (i != 0)
            {
                occ.erase(s[i-1]);
            }
            while(rk + 1 < n && !occ.count(s[rk + 1]))
            {
                occ.insert(s[rk + 1]);
                ++rk;
            }
            ans = max(ans, rk - i + 1);
        }
        return ans;
    }
};

int main(){
    Solution search_obj;
    string s = "abcabcbb";
    int num = search_obj.lengthOfLongestSubstring(s);
    cout << num << endl;
}

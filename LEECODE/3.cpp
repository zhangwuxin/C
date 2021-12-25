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
        int ans = 0;
        for (int left = 0, right = 0; right < n; ++right){
            while(occ.count(s[right]))
            {
                occ.erase(s[left])
                ++left;
            }
            occ.in
            ans = max(ans, rk - i + 1);
            cout << "ans" << ans << endl;
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

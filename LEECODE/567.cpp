#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;


class Solution {
public:
    bool checkInclusion(string s1, string s2) {
        int n = s1.length(), m = s2.length();
        if (n > m)
        {
            return false;
        }
        vector<int> cnt1(26), cnt2(26);
        for(int i = 0; i < n; i++)
        {
            ++cnt1[s1[i] - 'a'];
            ++cnt2[s2[i] - 'a'];
        }
        if(cnt1 == cnt2)
        {
            return true;
        }
        for(int i = n; i < m; i++)
        {
            ++cnt2[s2[i] - 'a'];
            --cnt2[s2[i-n] -'a'];
            if(cnt1 == cnt2 )
            {
                return true;
            }
        }
        return false;
    }
};

int main(){
    Solution search_obj;
    string s1 = "ab";
    string s2 = "eiabcdab";
    int num = search_obj.checkInclusion(s1,s2);
    cout << num << endl;
}

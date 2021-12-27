#include <string.h>
#include <vector>
#include <iostream>
#include <string>
using namespace std;

class Solution {
public:
    string reverseWords(string s) {
        int left = 0, right = 0;
        while(right < s.length())
        {
            int temp = right;
            if(s[right+1] == ' ' || right+1 == s.length())
            {
                while(left < right){
                    swap(s[left], s[right]);
                    left++;
                    right--;
                }
                left = right = temp+2;
            }else{
                right += 1;
            }
        }
        return s;
    }
};

int main(){
    Solution search_obj;
    string s = "Let's take LeetCode contest";
    s = search_obj.reverseWords(s);
    int i = 0;
    while(i < s.length())
    {
        cout << s[i] ;
        i++;
    }
}
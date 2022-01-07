#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;


class Solution {
public:
    vector<vector<int>> temp = {};
    vector<vector<int>> combine(int n, int k) {
        for(int i=1; i<k;i++)
        {
            for(int j=i+1;j<=k;j++)
            {
                temp.push_back({i,j});
            }
        }
        return temp;
    }
};

int main(){
    Solution search_obj;
    vector<vector<int>> A = search_obj.combine(1,4);
    for(auto e:A){
        for(auto x:e){
            cout << x << endl;
        }
        cout << endl;
    }
}

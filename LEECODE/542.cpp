#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;

class Solution
{
public:
    /* 不知道为啥超时了
    vector<vector<int>> updateMatrix(vector<vector<int>> &mat)
    {
        int m = mat.size(), n = mat[0].size();
        vector<vector<int>> dist(m, vector<int>(n, INT_MAX / 2));
        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < n; j++)
            {
                if (mat[i][j] == 0)
                {
                    dist[i][j] = 0;
                }
            }
        }
        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < n; j++)
            {
                if (i - 1 >= 0)
                {
                    dist[i][j] = min(dist[i][j], dist[i - 1][j] + 1);
                }
                if (j - 1 >= 0)
                {
                    dist[i][j] = min(dist[i][j], dist[i][j - 1] + 1);
                }
            }
        }
        for (int i = m - 1; i >= 0; i++)
        {
            for (int j = n - 1; j >= 0; j++)
            {
                if (i + 1 < m)
                {
                    dist[i][j] = min(dist[i][j], dist[i + 1][j] + 1);
                }
                if (j + 1 < n)
                {
                    dist[i][j] = min(dist[i][j], dist[i][j + 1] + 1);
                }
            }
        }
        return dist;
    } */
    vector<vector<int>> updateMatrix(vector<vector<int>> &mat)
    {
        int m = mat.size(), n = mat[0].size();
        vector<vector<int>> dist (m, vector<int>(n));
        vector<vector<int>> seen (m, vector<int>(n));
        queue<pair<int,int>> q;
        for(int i=0; i<m; i++)
        {
            for(int j=0;j<n;j++)
            {
                if(mat[i][j] == 0)
                {
                    q.emplace(i,j)
                    seen[i][j]=1;
                }
            }
        }
        while(!q.empty())
        {
            
        }
    }

};

int main()
{
    Solution search_obj;
    vector<vector<int>> X = {{0, 0, 0}, {0, 1, 0}, {1, 1, 1}};
    vector<vector<int>> A = search_obj.updateMatrix(X);
    for (auto e : A)
    {
        for (auto x : e)
        {
            cout << x;
        }
        cout << endl;
    }
}

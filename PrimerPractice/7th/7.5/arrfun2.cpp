#include <iostream>
const int ArSize = 8;
using namespace std;
// int sum_arr(int arr[], int n);
int sum_arr(int *arr, int n);
int main()
{
    
    int cookies[ArSize] = {1, 2, 4, 8, 16, 32, 64, 128};
    cout << cookies << " = array address , " << endl;
    //0 + 4*4 
    cout << cookies + 4 << " = cookies + 4 array address , " ;
    cout << sizeof(cookies) << " = sizeof cookies" << endl;
    int sum = sum_arr(cookies, ArSize);
    cout << "Total cookies eaten : " << sum << endl;
    sum = sum_arr(cookies, 3);
    cout << "First three eaters ate " << sum << " cookies." << endl;
    sum = sum_arr(cookies + 4, 4);
    cout << "Last four eaters ate " << sum << " cookies." << endl;
    return 0;
}

// int sum_arr(int arr[], int n)
int sum_arr(int *arr, int n)
{
    int total = 0;
    cout << arr << " = arr, ";
    cout << sizeof(arr) << " = sizeof arr" << endl;
    for (int i = 0; i < n; i++)
    {
        total = total + arr[i];
    }
    return total;
}

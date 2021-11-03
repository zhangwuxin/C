#include <iostream>
const int Max = 5;
using namespace std;
int fill_array(double ar[], int limit);
void show_array(const double ar[], int n);
void revalue(double r, double ar[], int n);
int main()
{
    using namespace std;
    double propretices[Max];
    
    
    int size = fill_array(propretices, Max);
    show_array(propretices, size);
    if (size > 0)
    {
        cout << "Enter revaluation factor : ";
        double factor;
        while (!(cin >> factor))
        {
            cin.clear();
            while (cin.get() != '\n')
            {
                continue;
            }
            cout << "bad input ; plase enter a number :";
        }
        revalue(factor, propretices, size);
        show_array(propretices, size);
    }
    cout << "Done.\n";
    cin.get();
    cin.get();
    return 0;
}

int fill_array(double ar[], int limit)
{
    using namespace std;
    double temp;
    int i;
    for ( i = 0; i < limit ; i++)
    {
        cout << "enter value # " << (i + 1) << ": ";
        cin >> temp;
        if(!cin)
        {
            cin.clear();
            while (cin.get() != '\n')
                continue;
            cout << "bad input ; input process ternminated.\n";
            break;
        }
        else if (temp < 0)
            break;
        ar[i] = temp;
    }
    return i;
}

void show_array(const double ar[], int n)
{
    using namespace std;
    for (int i = 0; i < n; i++)
    {
        cout << "property #" << (i + 1) << ": $";
        cout << ar[i] << endl;
    }
}

void revalue(double r, double ar[], int n)
{
    for (int i = 0; i < n; i++)
        ar[i] *= r;
}
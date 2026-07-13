#include <iostream>

int main()
{
    using std::cout;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 1; j <= 10; j++)
        {
            cout << j << " ";
        }
        cout << "\n";
    }

    return 0;
}
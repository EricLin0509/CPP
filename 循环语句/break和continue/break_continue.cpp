#include <iostream>

int main()
{
    using std::cout;

    for (int i = 0; i < 10; i++)
    {
        if (i == 5)
            break;
        cout << i << "\n";
    }
    cout << "Loop ended.\n";


    for (int i = 0; i < 10; i++)
    {
        if (i == 5)
            continue;
        cout << i << "\n";
    }
    cout << "Loop ended.\n";

    return 0;
}
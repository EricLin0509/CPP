#include <iostream>

int main()
{
    int arr[] = {1, 2, 3, 4, 5};

    for (int num : arr)
    {
        std::cout << num << " ";
    }
    std::cout << "\n";

    return 0;
}
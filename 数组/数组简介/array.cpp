#include <iostream>

int main()
{
    int arr[3] = {1, 2, 3};

    std::cout << arr << std::endl;

    std::cout << arr[0] << std::endl;
    std::cout << arr[1] << std::endl;
    std::cout << arr[2] << std::endl;

    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;

    std::cout << arr[0] << std::endl;
    std::cout << arr[1] << std::endl;
    std::cout << arr[2] << std::endl;

    return 0;
}
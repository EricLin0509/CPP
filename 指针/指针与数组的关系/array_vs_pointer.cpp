#include <iostream>

void printArray(int arr[]) // 实际等价于 void printArray(int* arr)
{
    std::cout << "Size of arr: " << sizeof(arr) << "\n"; // 此时是指针的大小
    for (int i = 0; i < 5; i++)
    {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";
}

int main() {
    int arr[5] = {1, 2, 3, 4, 5};
    std::cout << "Size of arr: " << sizeof(arr) << "\n";
    std::cout << "arr: " << arr << "\n";
    std::cout << "*arr: " << *arr << "\n";

    std::cout << "arr[2]: " << arr[2] << "\n";
    std::cout << "*(arr + 2): " << *(arr + 2) << "\n";
    std::cout << "2[arr]: " << 2[arr] << "\n";

    printArray(arr);

    return 0;
}
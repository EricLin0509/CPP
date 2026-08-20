#include <iostream>
#include <array>

void printArray(const std::array<int, 5>& arr)
{
    for (int i = 0; i < arr.size(); ++i)
    {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";
}

void printArray(int *arr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::array<int, 5> arr1 = {1, 2, 3, 4, 5};
    std::array<int, 5> arr2 = {6, 7, 8, 9, 10};

    arr1.swap(arr2);

    printArray(arr1);
    printArray(arr2);

    printArray(arr1.data(), arr1.size()); // 使用data()函数获取数组的指针

    arr1.fill(0);

    printArray(arr1);

    try
    {
        arr1.at(0) = 1; // 将第一个元素设置为 1
    }
    catch (const std::out_of_range& e)
    {
        std::cout << "[ERROR] Index out of range!" << std::endl;
    }

    printArray(arr1);

    return 0;
}

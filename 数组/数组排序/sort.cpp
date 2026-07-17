#include <iostream>

void quick_swap(int *a, int *b)
{
    if (a == NULL || b == NULL) return; // 防止空指针异常
    if (a == b) return; // 防止交换同一个元素

    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

int main()
{
    int arr[] = {10, 1, 5, 7, 8, 2, 4, 6, 3, 9};

    int n = sizeof(arr) / sizeof(arr[0]);

    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - 1 - i; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                quick_swap(arr + j, arr + j + 1);
            }
        }
    }

    for (int num : arr)
    {
        std::cout << num << " ";
    }
    std::cout << "\n";

    return 0;
}
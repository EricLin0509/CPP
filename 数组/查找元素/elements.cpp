#include <iostream>

int main()
{
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    int input = 0;
    std::cout << "请输入要查找的元素：";
    std::cin >> input; // 输入要查找的元素

    int index = 0; // 初始化索引

    for (index = 0; index < 10; index++) // 遍历数组
    {
        if (arr[index] == input)
        {
            break; // 找到元素，退出循环
        }
    }

    if (index == 10) // 正常退出for循环时，index为10，表示元素不存在
    {
        std::cout << "元素不存在" << std::endl; // 元素不存在
    }
    else
    {
        std::cout << "元素存在，索引为" << index << std::endl; // 元素存在，索引为index
    }

    return 0;
}
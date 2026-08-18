#include <iostream>
#include <vector>

using std::vector;

int main() {
    vector<int> vec;
    vec.reserve(10); // 预留空间

    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    vec.push_back(5);

    std::cout << "Size: " << vec.size() << "\n"; // 输出向量大小
    std::cout << "Capacity: " << vec.capacity() << "\n"; // 输出向量容量

    try
    {
        std::cout << vec.at(6) << "\n"; // 这里会抛出异常
    }
    catch (const std::out_of_range& e)
    {
        std::cout << "索引超出范围" << "\n";
    }

    vec.insert(vec.begin() + 5, 100);

    vec.erase(vec.begin() + 4);

    for (int i = 0; i < vec.size(); i++)
    {
        std::cout << vec[i] << " "; // 输出向量所有元素
    }
    std::cout << "\n";

    vec.clear();

    return 0;
}
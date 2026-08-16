#include <iostream>
#include <vector>

using std::vector;

int main() {
    vector<int> vec = {1, 2, 3, 4, 5};

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
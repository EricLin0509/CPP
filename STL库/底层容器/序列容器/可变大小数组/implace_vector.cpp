#include <iostream>
#include <implace_vector>

int main() {
    std::implace_vector<int, 5> vec;

    try
    {
        vec.at(0) = 1; // 将第一个元素设置为 1
    }
    catch (const std::out_of_range& e)
    {
        std::cout << "[ERROR] Index out of range!" << std::endl;
    }

    vec.try_push_back(3); // 尝试在向量尾部插入元素 3
    vec.try_emplace_back(2); // 尝试在向量尾部插入元素 2
}
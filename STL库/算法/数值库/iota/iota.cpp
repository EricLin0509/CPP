#include <iostream>
#include <algorithm>
#include <numeric>
#include <vector>

int main() {
    std::vector<int> vec(10);
    std::iota(vec.begin(), vec.end(), 0);
    for (int i : vec)
    {
        std::cout << i << " ";
    }
    std::cout << "\n";

    std::vector<int> data = {4, 3, 2, 1};
    std::vector<int> indices(data.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](int a, int b) {
        return data[a] < data[b]; // 倒序排序
    });

    for (int i : indices)
    {
        std::cout << "data[" << i << "] = " << data[i] << "\n";
    }

    return 0;
}

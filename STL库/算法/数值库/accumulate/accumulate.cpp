#include <iostream>
#include <numeric>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};

    // 默认求和
    int sum = std::accumulate(v.begin(), v.end(), 0);
    std::cout << "求和: " << sum << "\n";

    // 求积
    int product = std::accumulate(v.begin(), v.end(), 1, std::multiplies<int>());
    std::cout << "求积: " << product << "\n";

    // 拼接字符串
    std::vector<std::string> words = {"Hello", " ", "World", "!"};
    std::string result = std::accumulate(words.begin(), words.end(), std::string(""));
    std::cout << "拼接: " << result << "\n";

    // 求最大值
    int max_val = std::accumulate(v.begin(), v.end(), v[0],
        [](int a, int b) { return std::max(a, b); });
    std::cout << "最大值: " << max_val << "\n";

    // 计数满足条件的元素（负数个数）
    std::vector<int> v2 = {1, -2, 3, -4, 5};
    int neg_count = std::accumulate(v2.begin(), v2.end(), 0,
        [](int count, int x) { return count + (x < 0 ? 1 : 0); });
    std::cout << "负数个数: " << neg_count << "\n";

    // 初始值决定返回类型
    std::vector<int> v3 = {100, 200, 300};
    double double_sum = std::accumulate(v3.begin(), v3.end(), 0.0);
    std::cout << "double求和: " << double_sum << "\n";

    // 空范围
    std::vector<int> empty;
    int empty_result = std::accumulate(empty.begin(), empty.end(), 42);
    std::cout << "空范围: " << empty_result << "\n";

    return 0;
}
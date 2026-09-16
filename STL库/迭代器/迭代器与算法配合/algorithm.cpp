#include <iostream>
#include <algorithm>
#include <list>
#include <vector>
#include <ranges>
#include <numeric>

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5};

    std::cout << std::distance(vec.begin(), vec.end()) << "\n"; // O(1)，直接做指针减法

    // std::find：查找元素
    auto it = std::find(vec.begin(), vec.end(), 3);
    if (it != vec.end())
    {
        std::cout << "找到: " << *it << "\n"; // 输出: 找到: 3
    }

    // std::count：统计元素出现次数
    int n = std::count(vec.begin(), vec.end(), 1);
    std::cout << "1 出现了 " << n << " 次\n"; // 输出: 1 出现了 1 次

    // std::accumulate：累加（需要 <numeric>）
    int sum = std::accumulate(vec.begin(), vec.end(), 0);
    std::cout << "总和: " << sum << "\n"; // 输出: 总和: 15

    // std::replace：替换元素
    std::replace(vec.begin(), vec.end(), 3, 99);
    // vec 为 {1, 2, 99, 4, 5}

    // std::transform：变换元素，写入到另一个容器
    std::vector<int> result(5);
    std::transform(vec.begin(), vec.end(), result.begin(), [](int x) { return x * 2; });
    // result 为 {2, 4, 198, 8, 10}

    // std::copy + back_inserter：复制到动态增长的容器
    std::vector<int> dest;
    std::copy(vec.begin(), vec.end(), std::back_inserter(dest));
    // dest 为 {1, 2, 99, 4, 5}

    // std::sort：排序
    std::sort(vec.begin(), vec.end());
    // vec 为 {1, 1, 2, 3, 4, 5, 6, 9}

    // std::sort 自定义比较
    std::sort(vec.begin(), vec.end(), std::greater<int>());
    // vec 为 {9, 6, 5, 4, 3, 2, 1, 1}

    // std::nth_element：第 n 小的元素放到正确位置
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};
    std::nth_element(v.begin(), v.begin() + 3, v.end());
    // v[3] 为 3（第 4 小的元素），其左侧都 ≤ 3，右侧都 ≥ 3

    // std::partial_sort：部分排序
    std::vector<int> v2 = {3, 1, 4, 1, 5, 9, 2, 6};
    std::partial_sort(v2.begin(), v2.begin() + 3, v2.end());
    // v2 前 3 个为最小的 3 个元素且有序：{1, 1, 2, ...}

    std::vector<int> vec_views = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // 取偶数，平方，取前 3 个
    auto result_views = vec_views
        | std::views::filter([](int x) { return x % 2 == 0; })
        | std::views::transform([](int x) { return x * x; })
        | std::views::take(3);

    // result 为 {4, 16, 36}（惰性求值，不产生中间容器）
    for (int x : result_views)
    {
        std::cout << x << " ";
    }
    std::cout << "\n";
}
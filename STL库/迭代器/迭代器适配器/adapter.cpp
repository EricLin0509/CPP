#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <iterator>

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5};

    auto rit = vec.rbegin();
    std::cout << *rit << "\n"; // 输出: 5
    std::cout << *(rit.base() - 1) << "\n"; // 输出: 5

    for (auto it = vec.rbegin(); it != vec.rend(); ++it)
    {
        std::cout << *it << " "; // 输出: 5 4 3 2 1
    }
    std::cout << "\n";

    /* rbegin() == base() - 1 */
    

    std::vector<int> src = {1, 2, 3};
    std::vector<int> dest;

    // 错误写法：dest 为空，std::copy 会越界
    // std::copy(src.begin(), src.end(), dest.begin());

    // 正确写法：使用 back_inserter 动态插入
    std::copy(src.begin(), src.end(), std::back_inserter(dest)); // dest 现在为 {1, 2, 3}

    for (int i : dest)
    {
        std::cout << i << " ";
    }
    std::cout << "\n";

    std::vector<int> src1 = {10, 20, 30};
    std::vector<int> dest1 = {1, 2, 3};

    auto it = std::inserter(dest1, dest1.begin() + 1); // 在位置 1 处插入
    std::copy(src1.begin(), src1.end(), it);
    // dest 为 {1, 10, 20, 30, 2, 3}

    for (int i : dest1)
    {
        std::cout << i << " ";
    }
    std::cout << "\n";

    std::list<int> src2 = {1, 2, 3};
    std::deque<int> dest2;

    std::copy(src2.begin(), src2.end(), std::front_inserter(dest2));
    // dest 为 {3, 2, 1}（每次 push_front，顺序反转）
    for (int i : dest2)
    {
        std::cout << i << " ";
    }
    std::cout << "\n";

    std::vector<std::string> src3 = {"hello", "world"};
    std::vector<std::string> dest3;

    // 使用 move_iterator 移动元素而非拷贝
    dest3.insert(dest3.end(),
        std::make_move_iterator(src3.begin()),
        std::make_move_iterator(src3.end()));
    // src3 中的字符串已被移动，可能变为空字符串
    // auto it = src3.begin();
    // std::cout << *it << "\n"; // 属于未定义行为

    for (std::string s : dest3)
    {
        std::cout << s << " ";
    }
    std::cout << "\n";

    return 0;
}
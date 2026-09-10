#include <iostream>
#include <iterator>

template <typename T>
class FibonacciIterator {
    private:
        T curr;
        T prev;
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::forward_iterator_tag;
};

int main() {
    FibonacciIterator<int> it;
    std::distance(it, it);
}
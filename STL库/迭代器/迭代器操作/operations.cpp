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

        FibonacciIterator()
        {
            curr = 1;
            prev = 0;
        }

        T operator*() const // 解引用操作符
        {
            return curr;
        }

        FibonacciIterator& operator++() // 前缀自增操作符 ++it
        {
            T temp = curr;
            curr = prev + curr;
            prev = temp;
            return *this;
        }

        FibonacciIterator operator++(int) // 后缀自增操作符 it++
        {
            FibonacciIterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator!=(const FibonacciIterator& other) const // 不等于操作符
        {
            return this->curr != other.curr;
        }

        bool operator==(const FibonacciIterator& other) const // 等于操作符
        {
            return this->curr == other.curr;
        }
};

int main() {
    FibonacciIterator<int> it;
    
    for (int i = 0; i < 10; i++)
    {
        std::cout << *it << " ";
        it++;
    }
    std::cout << "\n";

    std::advance(it, 5);
    std::cout << *it << "\n";
}

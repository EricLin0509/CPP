#include <iostream>
#include <memory>

int main() {
    auto ptr = std::make_shared<int>(10);
    auto ptr1 = ptr;
    auto ptr2 = ptr;

    return 0;
}
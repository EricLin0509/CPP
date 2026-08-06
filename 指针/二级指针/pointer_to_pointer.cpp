#include <iostream>

int main() {
    int a = 10;
    int *p = &a;
    int **q = &p;

    std::cout << "Pointer p: " << p << "\n";
    std::cout << "Pointer q: " << q << "\n";
    std::cout << "Value of *q: " << *q << "\n";

    std::cout << "Value of *p: " << *p << "\n";
    std::cout << "Value of **q: " << **q << "\n";

    return 0;
}
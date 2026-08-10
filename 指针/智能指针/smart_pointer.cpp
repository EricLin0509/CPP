#include <iostream>
#include <memory>

class Object {
    public:
        Object()
        {
            std::cout << "Object created\n";
        }

        ~Object()
        {
            std::cout << "Object destroyed\n";
        }

        void print()
        {
            std::cout << "Hello World from Object\n";
        }
};

int main() {
    std::unique_ptr<Object> ptr1 = std::make_unique<Object>();
    ptr1->print();

    std::shared_ptr<Object> ptr2 = std::make_shared<Object>();
    std::shared_ptr<Object> ptr3 = ptr2;

    std::cout << "use_count: " << ptr2.use_count() << "\n";
    ptr3->print();

    std::weak_ptr<Object> ptr4 = ptr2;
    std::cout << "use_count: " << ptr2.use_count() << "\n";

    std::shared_ptr<Object> shared = ptr4.lock();
    std::cout << "use_count: " << ptr2.use_count() << "\n";
    if (shared)
        shared->print();

    return 0;
}
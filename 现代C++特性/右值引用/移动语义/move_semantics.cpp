#include <iostream>
#include <string.h>

class MyString {
    private:
        char *str;
    public:
        MyString(const char *src)
        {
            str = new char[strlen(src) + 1];
            strcpy(str, src);
            std::cout << "MyString(const char *src) called" << "\n";
        }
        MyString(MyString &other)
        {
            str = new char[strlen(other.str) + 1];
            strcpy(str, other.str);
            std::cout << "MyString(MyString &other) called" << "\n";
        }
        MyString(MyString &&other)
        {
            str = other.str;
            other.str = nullptr;
            std::cout << "MyString(MyString &&other) called" << "\n";
        }
        ~MyString()
        {
            delete[] str;
            std::cout << "Object deleted\n";
        }

        MyString &operator=(MyString &&other) noexcept
        {
            if (this == &other) return *this;

            delete[] str;
            str = other.str;
            other.str = nullptr; // 移动资源
            return *this;
        }
};

int main() {
    MyString str1("Hello");
    MyString str2 = std::move(str1); // 移动语义

    return 0;
}
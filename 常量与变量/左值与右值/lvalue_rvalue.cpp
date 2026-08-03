#include <iostream>

int globalVar = 10;
int &func()
{
    return globalVar;
}

class Dog {
    private:
        std::string sound;
    public:
        Dog() { }

        void makeSound()
        {
            sound = "woof";
        }
};

int main() {
    func() = 20;

    int arr[10];
    arr[2] = 4; // `[]` 永远生成一个左值

    Dog().makeSound(); // 这就修改了右值中的成员变量

    return 0;
}
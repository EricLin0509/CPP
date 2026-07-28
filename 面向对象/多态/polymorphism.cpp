#include <iostream>

class OperatingSystem {
    public:
        virtual void getInfo() {
            std::cout << "It's running an operating system." << "\n";
        }
};

class Linux : public OperatingSystem {
    public:
        void getInfo() override {
            std::cout << "It's running Linux." << "\n";
        }

        void installSoftware() {
            std::cout << "Installing software on Linux." << "\n";
        }
};

class Windows : public OperatingSystem {
    public:
        void getInfo() override {
            std::cout << "It's running Windows." << "\n";
        }

        void installSoftware() {
            std::cout << "Installing software on Windows." << "\n";
        }
};

int main() {
    Linux computer1;
    Windows computer2;
    Linux computer3;

    OperatingSystem* computers[] = {&computer1, &computer2, &computer3};

    for (OperatingSystem* computer : computers)
    {
        computer->getInfo(); // 调用各自重写的虚函数
    }

    // computer1.installSoftware(); // 这行代码会报错，动态多态无法访问子类特有的成员函数

    return 0;
}
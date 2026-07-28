# 多态

在C++中，多态 (Polymorphism) 是指同一个接口可以有不同的实现方式

多态可以分为两种

- 动态多态: 通过虚函数来实现
- 静态多态: 通过函数重载或函数指针来实现

## 示例

### 动态多态

动态多态是在程序运行时根据对象的实际类型来调用相应的函数

现在有个类 `OperatingSystem`，它有一个虚函数 `getInfo`，不同的操作系统有不同的实现方式

```cpp
class OperatingSystem {
    public:
        virtual void getInfo() {
            std::cout << "It's running an operating system." << "\n";
        }
};
```

现在我们创建一个个子类 `Linux`

```cpp
class Linux : public OperatingSystem {
    public:
        void getInfo() override {
            std::cout << "It's running Linux." << "\n";
        }

        void installSoftware() {
            std::cout << "Installing software on Linux." << "\n";
        }
};
```

创建一个 `Linux` 对象并调用 `getInfo` 方法

```cpp
Linux linux;
linux.getInfo();
```

此时会输出 `It's running Linux.`

现在创建一个 `OperatingSystem` 指针并指向 `Linux` 对象

```cpp
Linux linux;
OperatingSystem* os = &linux;
os->getInfo();
```

此时同样会输出 `It's running Linux.`

这就是动态多态的实现方式——用父类的指针执行子类的虚方法

但**不能访问子类特有**的成员函数

```cpp
os->installSoftware(); // 错误，无法访问子类特有的成员函数
```

```
polymorphism.cpp:25:9: error: ‘class OperatingSystem’ has no member named ‘installSoftware’
```

#### 常见用法

用一个数组/列表保存不同的子类 (但他们都**继承自同一个基类**) 并批量操作一些共有的方法 (重写父类的虚函数)

```cpp
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

    OperatingSystem* computers[] = {&computer1, &computer2, &computer3}; // 这就是动态多态

    for (OperatingSystem* computer : computers)
    {
        computer->getInfo(); // 调用各自重写的虚函数
    }

    return 0;
}
```

### 静态多态

静态多态是在编译时根据函数的参数类型来调用相应的函数

有两种方式实现静态多态

- [函数重载](../../函数/函数重载/README.md)
- [操作符重载](../操作符重载/README.md)

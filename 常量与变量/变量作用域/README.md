# 变量作用域

变量作用域 (variable scope) 是指变量在程序中可以被访问的范围。

变量作用域可以分为以下几种：


- 局部变量 (local variable)
    - 局部变量只能在定义它的函数或块中被访问
    - 局部变量的生命周期是函数或块的生命周期
- 全局变量 (global variable)
    - 全局变量在程序的任何地方都可以被访问
    - 全局变量的生命周期是整个程序的生命周期

## 局部变量

局部变量只能在定义它的函数或块中被访问

假设 `main` 函数中定义了一个局部变量 `myNum`, 尝试在函数 `myFunc` 中访问 `myNum`

```cpp
void myFunc();

int main()
{
    int myNum = 10; // 局部变量

    myFunc();

    return 0;
}

void myFunc()
{
    std::cout << myNum << std::endl;
}
```

此时会发生报错

```
scope.cpp:16:18: error: ‘myNum’在此作用域中尚未声明
```

这是因为 `myNum` 只能在 `main` 函数中被访问，`myFunc` 函数中无法访问 `myNum`

在不同的函数或块中定义的局部变量具有不同的作用域

```cpp
void myFunc();

int main()
{
    int myNum = 10; // 局部变量

    myFunc();

    return 0;
}

void myFunc()
{
    int myNum = 20; // 局部变量

    std::cout << myNum << std::endl;
}
```

此时输出结果为 `20`，因为 `myNum` 在 `myFunc` 函数中被定义，`myNum` 的值为 `20`

## 全局变量

全局变量在程序的任何地方都可以被访问

假设定义一个全局变量 `gloNum`, 在 `main` 函数和 `myFunc` 函数中都可以访问 `gloNum`

```cpp
int gloNum = 10; // 全局变量

void myFunc();

int main()
{
    std::cout << gloNum << std::endl;

    myFunc();

    return 0;
}

void myFunc()
{
    std::cout << gloNum << std::endl;
}
```

此时输出结果为 `10`，因为 `gloNum` 在程序的任何地方都可以被访问

- **注意**: 一般情况下不推荐使用全局变量，因为可能会污染全局命名空间，导致命名冲突

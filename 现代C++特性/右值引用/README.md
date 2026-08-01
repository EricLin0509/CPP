# 右值引用

之前 [引用传递](../../函数/引用传递/README.md) 中了解怎么修改传入的参数，但那属于**左值引用**

右值引用（Rvalue Reference）是C++11引入的一种特性

常用于以下场景：

- [移动语义 (Move Semantics)](移动语义/README.md): 用于优化资源管理，减少内存拷贝
- 完美转发 (Perfect Forwarding): 用于传递函数参数时，保持参数的原始性

右值引用允许函数接收临时对象或右值，从而避免不必要的拷贝

## 基本语法

使用 `&&` 符号来声明右值引用

```cpp
int&& r = 10; // r 是一个右值引用
```

## 简单示例

有两个版本的 `printInt` 函数，分别接收左值和右值

```cpp
void printInt(int& r) { std::cout << r << "\n"; }
void printInt(int&& r) { std::cout << r << "\n"; }
```

```cpp
int a = 10;
printInt(a); // 调用 printInt(int&)，传入左值
printInt(10); // 调用 printInt(int&&)，传入右值
```


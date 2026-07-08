# 类型转换

类型转换是将一个数据类型转换为另一个数据类型的过程

在C++中，类型转换主要有两种方式：隐式类型转换和显式类型转换

## 隐式类型转换

隐式类型转换是指在编译器自动进行的类型转换

例如，用一个整数给 `char` 变量赋值，编译器会自动将整数转换为 `char` 类型

```cpp
char c = 65;
std::cout << c << std::endl; // 输出 A
```

## 显式类型转换

显式类型转换是指在程序中手动进行的类型转换

写法如下

```cpp
(<类型名>) <表达式>
```

例如，我们用两个整数算一个百分比

```cpp
int a = 80;
int b = 100;
double percent = a / b * 100;
std::cout << percent << "%" << std::endl;
```

但这样会得到 `0%`，因为 `a` 和 `b` 都是整数，所以 `a / b` 的结果会被截断为 `0`

为了得到正确的百分比，我们可以将 `a` 或 `b` 转换为 `double` 类型

```cpp
int a = 80;
int b = 100;
double percent = (double) a / (double) b * 100;
std::cout << percent << "%" << std::endl;
```

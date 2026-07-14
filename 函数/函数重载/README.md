# 函数重载

在C++中，函数重载是指在同一个作用域内，定义多个具有相同名称但参数列表不同的函数

## 原理

在编译器处理函数重载时，会根据参数列表生成一个唯一的函数签名，然后在函数调用时，会根据函数签名来选择合适的函数

假设我们定义了这两个函数

```cpp
void print_num(int a);
void print_num(double a);
```

在编译器处理函数重载时，会生成两个不同的函数签名

- _Z9print_numi: `void print_num(int a)`
- _Z9print_numd: `void print_num(double a)`

## 示例

设计一组加法函数，处理不同数量的参数

```cpp
int add(int a, int b)
{
    return a + b;
}

int add(int a, int b, int c)
{
    return a + b + c;
}

int add(int a, int b, int c, int d)
{
    return a + b + c + d;
}
```

```cpp
add(1, 2); // 3
add(1, 2, 3); // 6
add(1, 2, 3, 4); // 10
```


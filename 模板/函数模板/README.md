# 函数模板

函数模板是C++中的一种特性，它允许我们定义一个通用的函数，而不需要为每个数据类型编写单独的函数

编译器在编译时会根据实际的参数类型根据模板参数自动推导出实际的参数类型，并生成相应的函数

函数模板使用`template`关键字来声明

## 语法

```cpp
template <typename 类型名>
返回类型名 函数名(参数列表)
{
    // 函数体
}
```

- `typename` 关键字用于指定模板参数的类型
    - 这里也可以使用 `class` 关键字 (此处与 `typename` 关键字功能相同)

## 示例

假设现在设计一个函数 `max`，用于比较两个数并返回较大的那个数

```cpp
int max(int a, int b)
{
    return a > b ? a : b;
}
```

如果比较的是两个 `int` 类型的变量这里没有问题

```cpp
int a = 10;
int b = 20;
int result = max(a, b);
```

但如果比较的是两个 `double` 类型的变量

```cpp
double a = 10.5;
double b = 20.5;
double result = max(a, b);
```

此时会发生编译错误，因为 `max` 函数没有为 `double` 类型的参数编写专门的函数

这里可以再设计一个为 `double` 类型参数编写专门的函数

```cpp
int max_int(int a, int b)
{
    return a > b ? a : b;
}

double max_double(double a, double b)
{
    return a > b ? a : b;
}
```

但如果还需要比较两个 `float`、`char` 、`size_t` 等类型的数据，这样会显得非常繁琐

这时可以使用函数模板来解决这个问题

### 使用函数模板

```cpp
template <typename T>
T max(T a, T b)
{
    return a > b ? a : b;
}
```

现在可以使用 `max` 函数模板来比较两个 `int`、`double`、`float`、`char` 等类型的数据

```cpp
int a = 10;
int b = 20;
int result1 = max(a, b); // result = 20
std::cout << "Max of " << a << " and " << b << " is " << result1 << "\n";

double c = 10.5;
double d = 20.5;
double result2 = max(c, d); // result = 20.5
std::cout << "Max of " << c << " and " << d << " is " << result2 << "\n";
```

### 底层原理

有关底层原理的详细信息，请参考[底层原理](../README.md#底层原理)

### 显式指定参数类型

有关显式指定参数类型，请参考[显式指定参数类型](../README.md#显式指定参数类型)

### 匹配多个参数

有关匹配多个参数，请参考[匹配多个参数](../README.md#匹配多个参数)

# 模板

模板 (Template) 是 C++ 中一种强大的特性，它允许你在编写代码时使用通用的类型和操作

在编译时，编译器会根据实际使用的类型生成具体的代码，从而实现代码的复用和类型安全

## 与函数似宏 (Function-like Macro) 的区别

| 特征 | 模板 | 函数似宏 |
| :--: | :--: | :--: |
| 处理时机 | 编译时 | 预处理时 |
| 类型安全 | 是 | 否 (宏只做字符串替换) |
| 运算符重载 | 支持 | 不支持 |
| 代码生成 | 带有类型信息的函数 | 内建代码 (相当于 `inline` 函数) |
| 参数求值次数 | 只求值一次 | 可能多次求值 (如 `MAX(a++, b++)`) |
| 调试难度 | 较低 | 较高 (只能指向以展开的代码) |

## 简单示例

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

现在通过 `objdump` 命令查看程序的函数符号表

```bash
objdump -t function_template | grep max
```

输出结果如下

```
00000000000012fa  w    F .text  0000000000000028              _Z3maxIdET_S0_S0_
00000000000012de  w    F .text  000000000000001c              _Z3maxIiET_S0_S0_
```

- `_Z3maxIiET_S0_S0_` 对应于 `int` 版本的 `max` 函数
- `_Z3maxIdET_S0_S0_` 对应于 `double` 版本的 `max` 函数

### 显式指定参数类型

有时候编译器可能无法自动推导出实际的参数类型，这时可以手动指定参数类型

```cpp
int result1 = max<int>(a, b); // result = 20
double result2 = max<double>(c, d); // result = 20.5
```

### 匹配多个参数

函数模板也可以匹配多个参数。使用 `,` 分隔多个参数类型

```cpp
template <typename T1, typename T2>
size_t max_size(T1 a, T2 b)
{
    return sizeof(a) > sizeof(b) ? sizeof(a) : sizeof(b);
}
```

```cpp
int e = 10;
double f = 20.5;
size_t result = max_size(e, f); // result = 8

double g = 10.5;
size_t result2 = max_size(g, f); // result = 8
```

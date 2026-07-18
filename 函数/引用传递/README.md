# 引用传递

在C++中，引用传递是一种传递方式，

它允许函数在调用时修改传递给它的参数的值。引用传递通常使用`&`符号来表示

## 示例

假设有两个值 `a` 和 `b`，我们希望在函数中交换它们的值

在一般情况下

```cpp
int a = 10;
int b = 20;

/* 交换 */
a ^= b;
b ^= a;
a ^= b;

std::cout << "a: " << a << std::endl;
std::cout << "b: " << b << std::endl;
```

可以正常交换

```
a: 20
b: 10
```

现在设计一个函数 `swap`，它接受两个参数 `int a` 和 `int b`，并交换它们的值

```cpp
void swap(int a, int b)
{
    a ^= b;
    b ^= a;
    a ^= b;
}
```
```cpp
swap(a, b);
std::cout << "a: " << a << std::endl;
std::cout << "b: " << b << std::endl;
```

可是运行输出

```
a: 10
b: 20
```

这是因为我们使用的是值传递，函数内部的 `a` 和 `b` 是 `a` 和 `b` 的副本，它们的值不会影响到外部的 `a` 和 `b`

所以这里要使用引用传递

### 使用引用传递

```cpp
void swap(int &a, int &b)
{
    a ^= b;
    b ^= a;
    a ^= b;
}
```
```cpp
swap(a, b);

std::cout << "a: " << a << std::endl;
std::cout << "b: " << b << std::endl;
```

现在就可以正常交换了

```
a: 20
b: 10
```


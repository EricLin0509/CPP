# Namespaces

命名空间（namespace）是一种用于组织代码的机制

它允许你将相关的代码放在一个命名空间中，从而避免命名冲突

## 场景

假设现在我们要定义两个 `x` 变量，但它们的值不同

```cpp
int x = 10;
int x = 20;
```

此种情况会导致编译错误

```
error: ‘int x’重声明
```

为了解决这个问题，我们可以使用命名空间

## 声明命名空间

命名空间的声明使用 `namespace` 关键字

```cpp
namespace <命名空间名> {
    <代码>
    // 这里可以是变量、函数、类等
}
```

这里我们定义了一个名为 `myNamespace` 的命名空间

```cpp
namespace myNamespace {
    int x = 20;
}
```

## 使用命名空间

我们可以使用 `::` 符号来访问命名空间中的变量

```cpp
std::cout << myNamespace::x << std::endl;
```

- `::`: 作用域解析运算符 (Scope Resolution Operator)

或者使用 `using` 关键字

此时我们可以在当前作用域中使用命名空间中的变量，而不需要使用 `<命名空间名>::` 

```cpp
using namespace myNamespace;

std::cout << x << std::endl; // 此时不需要使用 myNamespace::x
```
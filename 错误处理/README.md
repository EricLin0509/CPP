# 错误处理

在C++中，错误处理是一个重要的方面，它可以帮助我们处理程序中的异常情况，避免程序崩溃

C++有以下几种错误处理机制：

- throw语句：用于抛出异常
- try-catch语句：用于捕获异常

## 语法

需要引入 `<exception>` 头文件

```cpp
#include <exception>
```

### throw语句

用于抛出异常

```cpp
throw std::exception{};
```

### try-catch语句

用于捕获异常

```cpp
try
{
    // 可能会抛出异常的代码
}
catch (std::exception& e)
{
    // 处理异常的代码
}
```

## 示例

假设现在有个函数 `divide`，它接受两个参数 `a` 和 `b`，并返回 `a` 除以 `b` 的结果

```cpp
double divide(int a, int b)
{
    return a / b;
}
```

但这样会有个问题，如果 `b` 为 0，那么就导致除零错误

为了解决这个问题，我们可以使用 `throw` 语句来抛出异常

### 使用 throw 语句

```cpp
double divide(int a, int b)
{
    if (b == 0)
    {
        throw std::exception{};
    }
    return a / b;
}
```

但如果此时我们直接调用这个函数并且 `b` 为 0，程序会崩溃

```cpp
double result = divide(10, 0);
```

```
terminate called after throwing an instance of 'std::exception'
  what():  std::exception
```

为了解决这个问题，我们可以使用 `try-catch` 语句来捕获异常

### 使用 try-catch 语句

可以使用 `what()` 方法来获取异常信息

```cpp
try
{
    double result = divide(10, 0);
}
catch (std::exception& e)
{
    std::cout << e.what() << "\n";
}
```

此时程序会输出错误信息，而不是崩溃

```
std::exception
```

## 自定义运行时异常

如果我们希望自定义运行时异常，需要创建一个类并继承自 `std::runtime_error` 类

```cpp
class DevideByZeroException : public std::runtime_error {

};
```

### 创建构造函数

这个构造函数需要调用基类的构造函数

```cpp
class DevideByZeroException : public std::runtime_error {
    public:
        DevideByZeroException() : std::runtime_error("Can not devide by zero") {} 
};
```

那么此时需要抛出并捕获的异常 `DevideByZeroException` 而不是 `std::exception`

```cpp
double divide(int a, int b)
{
    if (b == 0)
    {
        throw DevideByZeroException{};
    }
    return a / b;
}
```

```cpp
try
{
    double result = divide(10, 0);
}
catch (DevideByZeroException& e)
{
    std::cout << e.what() << "\n";
}
```

```
Can not devide by zero
```


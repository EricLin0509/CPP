# 完美转发

完美转发（Perfect Forwarding）是指允许包装函数模板将其参数传递给另一个函数，同时保留其精确的值类别 (左值或右值) 以及 const/volatile 修饰符

完美转发的关键在于使用 `std::forward` 函数，它能够正确地将参数传递给另一个函数，同时保留其精确的值类别

简单来说就是将参数原封不动地传递给下一个函数,保持其原始的左值/右值属性

## 引用折叠

在 C++11 中，引用折叠 (Reference collapsing) 是指在编译时，编译器会将引用折叠为一个单一的引用类型

转换规则如下表所示

| 原始类型 T | 附加引用 | 结果 |
| :-------: | :----: | :--: |
| T&        | &      | T&   |
| T&        | &&     | T&   |
| T&&       | &      | T&   |
| T&&       | &&     | T&&  |

- 规律是只有两者都为 `&&` 右值引用时，结果才为 `T&&`，否则为 `T&`

这个规律在完美转发中非常重要，因为 `std::forward` 函数需要知道参数的精确值类别，才能正确地传递给另一个函数

## 示例

现在有个函数模板 `wrapper`, 用于包装两个不同版本的函数 `process` (左值版本和右值版本)

```cpp
void process(int& x)
{
    std::cout << "Left value version: " << x << "\n";
}

void process(int&& x)
{
    std::cout << "Right value version: " << x << "\n";
}
```

```cpp
template<typename T>
void wrapper(T&& arg)
{
    process(arg);
}
```

但此时会有个问题：在函数体内，具名参数 `arg` 始终是左值表达式，因此 `process(arg)` 只能匹配 `process(int&)` 版本，无法调用 `process(int&&)` 版本

这样彻底抵消了移动语义带来的性能优势

为此，我们可以使用 `std::forward` 函数来完美转发参数

### 使用完美转发

使用 `std::forward` 函数，可以正确地将参数传递给另一个函数，同时保留其精确的值类别

```cpp
template<typename T>
void wrapper(T&& arg)
{
    process(std::forward<T>(arg));
}
```

`std::forward` 的工作原理
- 当 T 是 `int` 时，返回 `int&&` (右值)
- 当 T 是 `int&` 时，返回 `int&` (左值)

```cpp
wrapper(10); // 输出: Right value version: 10
int x = 10;
wrapper(x); // 输出: Left value version: 10
```

这里根据之前的 [引用折叠](#引用折叠) 规则，可以推导这两者的区别

- `wrapper(10)`: 实参是右值字面量
    - T 推导为 `int`
    - `T&&` = `int&&`
    - `std::forward<int>(arg)` 返回 `int&&` → 调用 `process(int&&)`
    
- `wrapper(x)`: 实参是左值变量
    - T 推导为 `int&`
    - `T&&` = `int& &&` = `int&` (引用折叠)
    - `std::forward<int&>(arg)` 返回 `int&` → 调用 `process(int&)`

## `std::move` VS `std::forward`

| 作用 | `std::move` | `std::forward` |
| :--: | :---------: | :------------: |
| 主要用途 | 将左值转换为右值 | 保留参数的精确值类别 |
| 使用场景 | 移动局部变量或表达式 | 包装函数内部的模板参数 |
| 模板类型 | 自动推导；无需显式指定 `<T>` | 必须显式提供模板参数 `<T>` |

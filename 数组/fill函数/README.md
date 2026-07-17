# fill函数

`fill`函数用于将数组中的所有元素设置为指定的值

通常用于初始化数组

## 语法

```cpp
std::fill(初始地址, 结束地址, 值);
```

## 示例

假设现在有一个长度为100的字符串数组 `arr`

```cpp
std::string arr[100];
```

如果我们要将数组中的所有元素设置为字符串 "Hello"

虽说可以使用显式初始化实现

```cpp
std::string arr[100] = {"Hello", "Hello", ..., "Hello"};
```

但这样会比较麻烦

因此我们可以使用 `fill` 函数来实现

### 使用 fill 函数

```cpp
std::fill(arr, arr + 100, "Hello");
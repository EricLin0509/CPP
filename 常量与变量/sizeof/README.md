# sizeof

sizeof 是一个运算符，用于计算变量或类型在内存中的大小

它返回一个无符号整数，表示所计算对象的字节数

## 语法

```cpp
sizeof(对象或类型)
```

## 示例

现有一个字符串变量 `str`，我们可以通过 `sizeof` 运算符来计算它的大小

```cpp
std::string str = "Hello World!";
std::cout << "The size of str is: " << sizeof(str) << " bytes" << std::endl;
```

输出结果为：

```
The size of str is: 32 bytes
```

这是因为 `std::string` 是引用类型，存储的是字符串的地址，而不是字符串的内容，所以它的大小是 32 字节
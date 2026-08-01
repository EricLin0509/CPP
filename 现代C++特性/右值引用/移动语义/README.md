# 移动语义

移动语义 (Move Semantics) 是指在C++中，当一个对象被移动时，其资源（如内存、文件句柄等）会被转移到另一个对象，而不是复制

移动语义可以提高性能，因为复制对象需要分配新的内存，而移动对象只需要复制指针或引用

## 示例

现在创建一个类 `MyString`

```cpp
class MyString {
    private:
        char *str;
    public:
        MyString(const char *src)
        {
            str = new char[strlen(src) + 1];
            strcpy(str, src);
            std::cout << "MyString(const char *src) called" << "\n";
        }
        MyString(const MyString &other)
        {
            str = new char[strlen(other.str) + 1];
            strcpy(str, other.str);
            std::cout << "MyString(MyString &other) called" << "\n";
        }
        ~MyString()
        {
            delete[] str;
            std::cout << "Object deleted\n";
        }
};
```

创建一个 `str1` 对象

```cpp
MyString str1("Hello");
```

现在利用 `str1` 创建一个 `str2` 对象

```cpp
MyString str2(str1);
```

但是这样会有性能问题，我们看一下输出

```
MyString(const char *src) called
MyString(MyString &other) called
Object deleted
Object deleted
```

那创建步骤如下：

1. `str1` 调用 `MyString(const char *src)` 构造函数，创建一个 `str1` 对象
2. `str2` 调用 `MyString(MyString &other)` 构造函数，创建一个 `str2` 对象并复制 `str1` 的资源
3. `str1` 调用 `~MyString()` 析构函数，删除 `str1` 对象
4. `str2` 调用 `~MyString()` 析构函数，删除 `str2` 对象

这里需要创建两个一模一样的对象，那有什么办法可以避免呢？我们可以利用移动语义

### 使用移动语义

使用右值引用实现移动语义

```cpp
class MyString {
    private:
        char *str;
    public:
        MyString(const char *src)
        {
            str = new char[strlen(src) + 1];
            strcpy(str, src);
            std::cout << "MyString(const char *src) called" << "\n";
        }
        MyString(const MyString &other)
        {
            str = new char[strlen(other.str) + 1];
            strcpy(str, other.str);
            std::cout << "MyString(MyString &other) called" << "\n";
        }
        MyString(MyString &&other) noexcept
        {
            str = other.str;
            other.str = nullptr; // 移动资源
            std::cout << "MyString(MyString &&other) called" << "\n";
        }
        ~MyString()
        {
            delete[] str;
            std::cout << "Object deleted\n";
        }

        MyString &operator=(MyString &&other) noexcept
        {
            if (this == &other) return *this;

            delete[] str;
            str = other.str;
            other.str = nullptr; // 移动资源
            return *this;
        }
};
```

- `operator=` 返回引用类型是为了支持链式赋值和符合 C++ 赋值运算符的惯例
    - 如果直接返回 `MyString` 类型，那么每次赋值操作都会返回一个副本，效率大大降低
- `noexcept`: 确保如 `Vector` 这样的容器不会在重新分配期间回退到复制操作

同时需要使用 `std::move` 函数将对象移动

```cpp
MyString str1("Hello");
MyString str2 = std::move(str1); // 移动语义
```

- **注意**: 此时 `str1` 已经被移动，**不能再使用**，否则会破坏对象的完整性，导致未定义行为

这样输出如下：

```
MyString(const char *src) called
MyString(MyString &&other) called
Object deleted
Object deleted
```

虽然仍然调用了两次析构函数，但避免了昂贵的内存分配和数据复制操作，这才是移动语义的性能优势所在

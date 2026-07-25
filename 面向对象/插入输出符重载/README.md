# 插入输出符重载

之前讲过如果在类中实现[操作符重载](../操作符重载/README.md)

同样的插入符 `<<` 和输出符 `>>` 也可以重载

## 语法

- 对于插入/输出符重载，**需要定义为非成员函数**
    - 由于通常需要访问类的私有成员，因此常声明为[友元函数](../友元函数/README.md)

### 插入符重载

```cpp
class 类名 {
    public:
        friend std::ostream& operator<<(std::ostream& os, 类名& obj)
        {
            // 具体实现
        }
};
```

- `ostream`: Output Stream，输出流，用于输出数据

### 输出符重载

```cpp
class 类名 {
    public:
        friend std::istream& operator>>(std::istream& is, 类名& obj)
        {
            // 具体实现
        }
};
```

- `istream`: Input Stream，输入流，用于输入数据


## 示例

现在有个类 `Person`

```cpp
class Person {
    private:
        std::string name;
    public:
        Person(std::string name)
        {
            this->name = name;
        }
};
```

如果我们希望直接使用 `cout` 输出 `Person` 对象，那么就需要重载插入符 `<<`

### 插入符重载

```cpp
class Person {
    private:
        std::string name;
    public:
        Person(std::string name)
        {
            this->name = name;
        }

        // 插入符重载
        friend std::ostream& operator<<(std::ostream& os, Person& p)
        {
            os << "Name: " << p.name << "\n";
            return os;
        }
};
```

此时可以直接使用 `std::cout` 输出 `Person` 对象

```cpp
Person p("John");
std::cout << p << "\n"; // 输出: Name: John
```

### 输出符重载

这里演示在类外部实现输出符重载

```cpp
class Person {
    private:
        std::string name;
    public:
        Person() {} // 默认构造函数

        Person(std::string name)
        {
            this->name = name;
        }

        // 插入符重载
        friend std::ostream& operator<<(std::ostream& os, Person& p)
        {
            os << "Name: " << p.name << "\n";
            return os;
        }

        friend std::istream& operator>>(std::istream& is, Person& p); // 只声明，不实现
};


std::istream& operator>>(std::istream& is, Person& p)
{
    std::cout << "Enter your name: ";
    is >> p.name;
    return is;
}
```

现在可以直接使用 `std::cin` 输入 `Person` 对象

```cpp
Person p;
std::cin >> p; // 输入: John
std::cout << p << "\n"; // 输出: Name: John
```


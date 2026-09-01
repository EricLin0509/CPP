# 模板特化

模板特化 (Template Specialization) 是指在模板定义中为特定类型提供特化版本

简单来说就是遇到特殊情况时需要单独处理，而不是使用通用的模板

例如说在已经提供可以处理基本类型的模板的情况下，如果需要处理如类这样的复杂类型，就需要提供特化版本

## 语法

### 主模板

主模板就是模板定义

模板特化首先需要一个主模板

```cpp
template <typename T>
返回值 函数名(T 参数)
{
    // 主模板的实现
}
```

### 模板特化

模板特化就是在主模板的基础上，为特定类型提供特化版本

```cpp
template <>
返回值 函数名(特定类型 参数)
{
    // 特化版本的实现
}
```

## 示例

现在有一个类 `Student`

```cpp
class Student
{
    private:
        std::string name;
        int age;

    public:
        Student(const char *name, int age)
        {
            this->name = name;
            this->age = age;
        }

        bool operator==(const Student &other) const
        {
            return this->name == other.name && this->age == other.age;
        }
};
```

现在我们希望使用 [`std::unordered_set`](../../STL库/底层容器/无序容器/无序集合/README.md) 来存储 `Student` 对象

```cpp
std::unordered_set<Student> students;
```

此时会报错

```bash
template_specialization.cpp:19:33: error: 使用了被删除的函数‘std::unordered_set<_Value, _Hash, _Pred, _Alloc>::unordered_set() [with _Value = Student; _Hash = std::hash<Student>; _Pred = std::equal_to<Student>; _Alloc = std::allocator<Student>]’
   19 |     std::unordered_set<Student> students;
      |                                 ^~~~~~~~
```

这是因为 `std::unordered_set` 中的 `std::hash` 模板无法处理 `Student` 类型

因此我们需要提供特化版本 (全特化)

### 全特化

全特化是指为特定类型提供特化版本，不使用任何模板参数

- `std::hash` 是标准库提供的模板，默认只支持基本类型 (如 `int`、`string` 等)
    - 对于自定义类型，需要提供特化版本

```cpp
class Student {
    private:
        std::string name;
        int age;

    public:
        Student(const char *name, int age)
        {
            this->name = name;
            this->age = age;
        }

        bool operator==(const Student &other) const
        {
            return this->name == other.name && this->age == other.age;
        }

        friend struct std::hash<Student>; // 使用友元函数使其能访问私有成员
};

/* 全特化版本 */
template <>
struct std::hash<Student> {
    size_t operator()(const Student &student) const
    {
        return std::hash<std::string>{}(student.name) ^ std::hash<int>{}(student.age);
    }
};
```

这样 `std::hash` 模板就可以正确处理 `Student` 类型了

### 偏特化

偏特化是指为特定类型提供特化版本，使用模板参数

> [!WARNING]
> 偏特化是 C++ 模板类独有的，函数模板没有

例如说单独处理所有指针类型

```cpp
/* 主模板 */
template <typename T>
class MyClass {
    private:
        T data;
};

/* 偏特化版本 */
template <typename T>
class MyClass<T*> {
    private:
        T *data;
};
```

#### 实例化

只有在创建对象时编译器才会根据模板参数实例化模板，生成真正的类

```cpp
MyClass<int> obj1; // 实例化主模板
MyClass<int*> obj2; // 实例化偏特化版本
```

## 模板匹配规则

编译器在选择模板版本时遵循以下优先级：

1. **全特化版本** (最优先)
2. **偏特化版本** (选择最特化的)
3. **主模板** (最后选择)

```cpp
/* 主模板 */
template <typename T>
class MyClass {
    private:
        T data;
};

/* 全特化版本 */
template <>
class MyClass<int[10]> {
    private:
        int data[10];
};


/* 偏特化版本 */
template <typename T>
class MyClass<T*> {
    private:
        T *data;
};
```

```cpp
MyClass<int[10]> obj1; // 全特化版本
MyClass<int*> obj2; // 偏特化版本
MyClass<int> obj3; // 主模板版本

std::cout << sizeof(obj1) << " bytes\n"; // 40 bytes
std::cout << sizeof(obj2) << " bytes\n"; // 8 bytes
std::cout << sizeof(obj3) << " bytes\n"; // 4 bytes
```

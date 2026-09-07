# 蜂巢

蜂巢 (`std::hive`) 是 C++26 新增的无序容器

它的前身是 `plf::colony`，经过了十几年的迭代，最终成为 C++26 的标准库容器

相比其他容器，它具有非常稳定的迭代器

## 内存布局

相比其他无序容器使用的哈希表，蜂巢使用了分块存储机制，每个块存储固定数量的元素

简单的C语言内存布局如下

```cpp
typedef struct Hive {
    char data[8];
    uint8_t skipfield[8]; // 跳跃字段
    size_t ref_count; // 引用计数
    size_t element_count; // 元素计数
    struct Hive* next;
    struct Hive* prev;
} Hive;
```

- 这里虽说 `data` 块是固定 8 字节，但实际 `std::hive` 是以实现定义的增长因子 (例如 1.69) 增长

具体的简单实现请参考 [hive_implementation.md](hive_implementation.md)

## 语法

需要引入 `hive` 头文件

```cpp
#include <hive>
```

### 声明

```cpp
std::hive<类型> 蜂巢名;
```

## 示例

现在声明一个存储整形的蜂巢

```cpp
std::hive<int> hive;
```
### 共同方法

有关所有无序容器都有的方法，请参考[无序容器的共同方法](../README.md#共同方法)

### `emplace()` 方法

`emplace()` 系列相比 [`insert()` 方法](../README.md#插入元素)可以避免不必要的拷贝和移动操作

```cpp
class Student {
    private:
        std::string name;
        int age;
    public:
        Student(std::string name, int age)
        {
            this->name = name;
            this->age = age;

            std::cout << "Constructor called\n";
        }
        Student(const Student& student)
        {
            this->name = student.name;
            this->age = student.age;

            std::cout << "Copy constructor called\n";
        }
        Student(Student&& student) noexcept
        {
            this->name = std::move(student.name);
            this->age = std::move(student.age);

            std::cout << "Move constructor called\n";
        }
};
```

```cpp
std::hive<Student> students;
Employee stu1("Alice", 25);
students.insert(stu1); // 调用拷贝构造函数
students.insert(std::move(stu1));
students.emplace("Bob", 28); // 调用移动构造函数
```

通过输出可以看到

```bash
Constructor called # 这个是 stu1 的构造函数
Copy constructor called
Move constructor called
Constructor called # 这个是使用 emplace() 方法构造的对象的构造函数
```

使用 `emplace()` 方法时效率最高，因为不需要拷贝和移动对象

### `emplace_hint()` 方法

`emplace_hint()` 方法与 `emplace()` 方法类似，但需要提供一个迭代器作为插入位置的提示

```cpp
auto it = students.insert(stu1);
students.emplace_hint(it, "Bob", 28);
```

### 容量

#### 返回当前存储空间能够容纳的元素数

使用 `capacity()` 方法可以返回当前存储空间能够容纳的元素数

```cpp
std::cout << "Capacity: " << hive.capacity() << "\n";
```

#### 预留存储空间

使用 `reserve()` 方法可以预先预留存储空间

```cpp
hive.reserve(100);
```

#### 释放未使用的内存

使用 `shrink_to_fit()` 方法可以释放未使用的内存

它尽可能将容量调整到实际使用的元素数

```cpp
hive.shrink_to_fit();
```

- 注意：`shrink_to_fit()` 可能会失效所有活跃元素的迭代器、指针或引用
    - 相比 `erase()` 只是被删除元素的迭代器失效，因为它是逻辑删除

#### 释放预留的内存块

使用 `trim_capacity()` 方法可以释放预留的内存块

相比 `shrink_to_fit()` 方法，`trim_capacity()` 只会释放空的内存块

```cpp
hive.trim_capacity();
```

- `trim_capacity()` 绝对不会失效任何活跃元素的迭代器、指针或引用

#### 重新分配活动块

使用 `reshape()` 方法可以重新分配活动块

相比 `shrink_to_fit()` 方法，`reshape()` 是动态调整容器未来以及当前内存块 (Blocks) 的大小上下限 (最小/最大容量限制)

```cpp
hive.reshape(100);
```

- 注意：`reshape()` 可能会失效所有活跃元素的迭代器、指针或引用

### 操作

#### 删除连续的重复元素

使用 `unique()` 方法可以删除连续的重复元素

```cpp
std::hive<int> hive = {1, 2, 2, 3, 4, 4, 5};
hive.unique(); // 结果为 {1, 2, 3, 4, 5}
```

#### 排序

使用 `sort()` 方法可以对元素进行排序

在 `std::hive` 中要求元素类型必须可比较

如果元素类型不支持比较，就需要使用自定义的比较函数

函数原型如下

```cpp
template <class T>
bool cmp(const T& a, const T& b);
```

- 也可以使用 lambda 表达式

```cpp
std::hive<int> hive = {3, 1, 4, 1, 5, 9, 2};
hive.sort(); // 升序
hive.sort(std::greater<int>()); // 降序
```

- `sort()` 默认使用 `std::less` 进行比较

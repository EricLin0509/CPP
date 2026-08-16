# 双向队列

双向队列 (`std::deque`) 是 STL 库中的一种底层容器

与 [队列 (`std::queue`)](../../容器适配器/队列/README.md) 不同，队列是容器适配器，需要一个底层容器来存储元素 (默认就是 `std::deque`)

与 [向量 (`std::vector`)](../向量/README.md) 相比，双向队列的内存布局是不连续的 (类似链表的内存布局)，因此在随机访问会比向量 (使用动态数组) 的效率低

## 与 `std::vector` 的区别

| 功能 | `std::vector` | `std::deque` |
| :--: | :-----------: | :----------: |
| 内存布局 | 连续内存 | [分段内存块](#内存布局) |
| 随机访问 | O(1) | O(1) 但稍慢 |
| 缓存友好性 | 极好 | 良好 (如果在同一个内存块内访问) |
| 在尾部插入/删除 | 极好 | 极好 |
| 在头部插入/删除 | 极差 (需要遍历向量并向后移动元素) | 极好 |
| C-API 兼容性 | 极好 (可以使用 &vec[0] 传递) | 没有任何兼容 |

## 内存布局

双向队列的内存布局是分段的，每个内存块都有一个固定大小，当内存块满了时，会创建一个新的内存块

以下是双向队列的内存布局简单示例 (C语言)

```c
#define CHUNK_SIZE 1024 // 每个内存块的大小为 1024 字节

struct Deque {
    void **map; // 指向内存块的指针数组
    size_t map_size; // 内存块指针数组的大小
    void *begin; // 指向第一个元素的指针
    void *end; // 指向最后一个元素的指针
};
```

- `map` 的实现机制与 `std::vector` 类似，都是动态数组，支持自动扩容

## 语法

需要引入 `deque` 头文件

```cpp
#include <deque>
```

### 声明双向队列

```cpp
std::deque<类型> 队列名;
```

## 示例

现在声明一个双向队列

```cpp
std::deque<int> deque;
```

### 检查是否为空

使用 `empty()` 方法检查双向队列是否为空

```cpp
std::cout << "Is deque empty: " << (deque.empty() ? "true" : "false") << "\n";
```

### 获取大小

使用 `size()` 方法获取双向队列的大小

```cpp
std::cout << "Size of deque: " << deque.size() << "\n";
```

### 普通方法添加元素

#### 从尾部添加元素

使用 `push_back()` 方法从尾部添加元素，时间复杂度为 O(1)

```cpp
deque.push_back(1);
```

#### 从头部添加元素

使用 `push_front()` 方法从头部添加元素，时间复杂度为 O(1)

```cpp
deque.push_front(0);
```

#### 插入元素

使用 `insert()` 方法在指定位置插入元素，时间复杂度为 O(n)

```cpp
deque.insert(deque.begin() + 1, 2);
```

### `emplace()` 系列方法添加元素

`emplace()` 系列方法相比 `insert()` 和 `push` 系列方法，可以避免不必要的拷贝和移动操作

这在配合对象构造函数时非常有用

```cpp
class Student {
    private:
        std::string name;
    public:
        Student(std::string name) // 直接构造
        {
            this->name = name;

            std::cout << "Constructor called\n";
        }
        Student(const Student &student) // 拷贝构造
        {
            this->name = student.name;

            std::cout << "Copy constructor called\n";
        }
        Student(Student &&student) // 移动构造
        {
            this->name = std::move(student.name);

            std::cout << "Move constructor called\n";
        }

        Student &operator=(Student &&student) noexcept // 移动赋值
        {
            if (this != &student)
                this->name = std::move(student.name);

            std::cout << "Move assignment called\n";

            return *this;
        }
};
```

```cpp
std::deque<Student> students;
Student s1("Alice");

students.insert(students.begin(), s1); // 使用拷贝构造
students.insert(students.begin(), std::move(s1)); // 使用移动构造
students.emplace(students.begin(), "Bob"); // 使用直接构造
```

- 相比[容器适配器的 `emplace` 方法](../../容器适配器/队列/README.md#emplace-方法)，必须提供拷贝赋值或移动赋值运算符
    - 因为 `std::deque` 在中间插入元素时需要移动已有元素 (依赖赋值操作)

通过输出可以看到

```bash
Constructor called # 这个是 `s1` 的构造函数
Copy constructor called
Move constructor called
Constructor called
```

- 使用 `emplace()` 方法添加的 `Student` 对象采用的是直接构造，效率最高

#### 从尾部添加元素

使用 `emplace_back()` 方法从尾部添加元素，时间复杂度为 O(1)

```cpp
students.emplace_back("Charlie");
```

#### 从头部添加元素

使用 `emplace_front()` 方法从头部添加元素，时间复杂度为 O(1)

```cpp
students.emplace_front("David");
```

#### 插入元素

使用 `emplace()` 方法在指定位置插入元素，时间复杂度为 O(n)

```cpp
students.emplace(students.begin() + 1, "Eve");
```

### 删除元素

#### 移除首元素

使用 `pop_front()` 方法删除首元素，时间复杂度为 O(1)

```cpp
deque.pop_front();
```

#### 移除末元素

使用 `pop_back()` 方法删除末元素，时间复杂度为 O(1)

```cpp
deque.pop_back();
```

#### 移除指定位置的元素

使用 `erase()` 方法删除指定位置的元素，时间复杂度为 O(n)

```cpp
deque.erase(deque.begin());
```

#### 清空双向队列

使用 `clear()` 方法清空**整个**双向队列，时间复杂度为 O(n)

```cpp
deque.clear();
```

### 访问元素

可以使用 `[]` 索引操作符访问元素，时间复杂度为 O(1)

```cpp
std::cout << "Element at index 0: " << deque[0] << "\n";
```

#### `at()` 方法

使用 `at()` 方法也可以访问元素，时间复杂度为 O(1)

相比 `[]` 索引操作符，`at()` 方法如果访问越界会抛出 `std::out_of_range` 异常

所以推荐配合 `try-catch` 块使用

```cpp
try
{
    std::cout << "Element at index 1: " << deque.at(1) << "\n";
}
catch (const std::out_of_range &e)
{
    std::cout << "Index out of range" << "\n";
}
```

### 遍历元素

可以直接使用 `for-each` 循环遍历双向队列

```cpp
for (int num : deque)
{
    std::cout << num << " ";
}
```

使用 `for` 循环遍历双向队列需要配合 `iterator`

```cpp
for (std::deque<int>::iterator it = deque.begin(); it != deque.end(); it++)
{
    std::cout << *it << " ";
}
```

## 迭代器失效

迭代器失效是指在双向队列进行插入或删除操作时，迭代器指向的元素被删除或移动，导致迭代器失效

```cpp
std::deque<int> deque = {1, 2, 3, 4, 5};

auto it = deque.begin();
deque.clear(); // 清空双向队列

std::cout << *it << "\n"; // 危险，这属于未定义行为
```

- 如果在迭代器失效后继续使用该迭代器，可能会导致程序崩溃或产生不可预测的结果

### 失效规则

| 操作 | 失效 |
| :--: | :--: |
| `push_front`/`push_back` | 所有迭代器可能失效 (包括 `end()`) |
| `emplace_front`/`emplace_back` | 所有迭代器可能失效 (包括 `end()`) |
| `insert`/`emplace` | 所有迭代器可能失效 (包括 `end()`) |
| `erase` | 如果在起始擦除——只有被擦除元素<br>如果在末尾擦除——只有被擦除元素和尾后迭代器<br>否则——所有迭代器 (包含尾后迭代器) |
| `clear` | 所有迭代器失效 |
| `pop_front`/`pop_back` | 被删除元素的迭代器 + `end()` 失效 |

- 可能失效是指不是每次调用都 100% 失效，而是说标准不保证任何迭代器保持有效，实现可以自由决定是否失效

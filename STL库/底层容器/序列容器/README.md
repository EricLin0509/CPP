# 序列容器

## 定义

序列容器是 C++ STL 库中的一种容器，它存储元素的顺序是固定的

## 序列容器的类型

- [`vector`](向量/README.md)：动态数组
- [`deque`](双向队列/README.md)：双向队列
- [`list`](双向链表/README.md)：双向链表
- [`forward_list`](单向链表/README.md)：单向链表
- `array`：固定大小数组

## 共同方法

> [!WARNING]
> `std::array` 只支持这些方法: [`empty()`](#empty-方法)、[`size()`](#size-方法)、[`begin()`](#begin-方法) 和 [`end()`](#end-方法)

所有序列容器都支持以下方法

- [`empty()`](#empty-方法)：检查容器是否为空
- [`size()`](#size-方法)：返回容器中元素的数量
- [`insert()`](#insert-方法)：在容器中插入元素 (在 `std::forward_list` 为 `insert_after()` 方法)
- [`emplace()`](#emplace-方法)：在容器中插入元素，同时构造元素 (在 `std::forward_list` 为 `emplace_after()` 方法)
- [`erase()`](#erase-方法)：从容器中删除元素
- [`clear()`](#clear-方法)：清空容器
- [`front()`](#front-方法)：返回容器中第一个元素
- [`back()`](#back-方法)：返回容器中最后一个元素 (`std::forward_list` **不支持**该方法)
- [`begin()`](#begin-方法)：返回容器中第一个元素的迭代器
- [`end()`](#end-方法)：返回容器中最后一个元素的迭代器

这里使用 `deque` 作为示例

### `empty()` 方法

使用 `empty()` 方法检查某个序列容器是否为空

```cpp
std::deque<int> deque;
std::cout << "Is deque empty: " << (deque.empty() ? "true" : "false") << "\n";
```

### `size()` 方法

使用 `size()` 方法返回某个序列容器中元素的数量

```cpp
std::deque<int> deque = {1, 2, 3, 4, 5};
std::cout << "Size of deque: " << deque.size() << "\n";
```

### 添加元素

以下是所有序列容器都可使用的添加元素的方法

#### `insert()` 方法

> [!WARNING]
> 在 `std::forward_list` 为 `insert_after()` 方法

使用 `insert()` 方法在某个序列容器中插入元素

```cpp
std::deque<int> deque = {1, 2, 3, 4, 5};
deque.insert(deque.begin() + 2, 6);
std::cout << "Deque after insertion: ";
for (int num : deque)
{
    std::cout << num << " ";
}
std::cout << "\n";
```

- `begin()`：返回容器中第一个元素的迭代器
    - 通过运算符可以在不同的位置插入元素

#### `emplace()` 方法

> [!WARNING]
> 在 `std::forward_list` 为 `emplace_after()` 方法

`emplace()` 系列相比 `insert()` 方法可以避免不必要的拷贝和移动操作

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

### 删除元素

以下是所有序列容器都可使用的删除元素的方法

#### `erase()` 方法

使用 `erase()` 方法从某个序列容器中删除元素

```cpp
std::deque<int> deque = {1, 2, 3, 4, 5};
deque.erase(deque.begin() + 2);
std::cout << "Deque after deletion: ";
for (int num : deque)
{
    std::cout << num << " ";
}
std::cout << "\n";
```

#### `clear()` 方法

使用 `clear()` 方法清空某个序列容器

```cpp
std::deque<int> deque = {1, 2, 3, 4, 5};
deque.clear();
```

### 访问元素

以下是所有序列容器都可使用的访问元素的方法

#### `front()` 方法

使用 `front()` 方法返回某个序列容器中第一个元素

```cpp
std::deque<int> deque = {1, 2, 3, 4, 5};
std::cout << "Front element: " << deque.front() << "\n";
```

#### `back()` 方法

> [!WARNING]
> `std::forward_list` **不支持**该方法

使用 `back()` 方法返回某个序列容器中最后一个元素

```cpp
std::deque<int> deque = {1, 2, 3, 4, 5};
std::cout << "Back element: " << deque.back() << "\n";
```

### 迭代器

以下是所有序列容器都可使用的迭代器方法

迭代器跟 `front()` 和 `back()` 方法不同，迭代器类似于指针，可以用来遍历容器中的元素

而 `front()` 和 `back()` 方法返回的是元素本身

#### `begin()` 方法

使用 `begin()` 方法返回某个序列容器中第一个元素的迭代器

```cpp
std::deque<int> deque = {1, 2, 3, 4, 5};
std::cout << "Deque elements: ";
for (int num : deque)
{
    std::cout << num << " ";
}
std::cout << "\n";
```

#### `end()` 方法

使用 `end()` 方法返回某个序列容器中最后一个元素的迭代器

```cpp
std::deque<int> deque = {1, 2, 3, 4, 5};
std::cout << "Deque elements: ";
for (int num : deque)
{
    std::cout << num << " ";
}
std::cout << "\n";
```

### 遍历容器

遍历容器的方法有多种，这里使用 `for` 循环和迭代器

#### `for` 循环

```cpp
std::deque<int> deque = {1, 2, 3, 4, 5};
std::cout << "Deque elements: ";
for (int num : deque)
{
    std::cout << num << " ";
}
std::cout << "\n";
```

#### 迭代器

```cpp
std::deque<int> deque = {1, 2, 3, 4, 5};
std::cout << "Deque elements: ";
for (std::deque<int>::iterator it = deque.begin(); it != deque.end(); ++it)
{
    std::cout << *it << " ";
}
std::cout << "\n";
```

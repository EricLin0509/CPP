# 智能指针

在C++中，智能指针是一种用于管理动态分配内存的工具

它能够自动管理内存的分配和释放，从而避免内存泄漏和资源竞争等问题

智能指针主要有以下几种

- `std::unique_ptr`: 用于管理单个对象，不能复制，只能移动
- `std::shared_ptr`: 用于管理多个对象，可以复制和移动
- `std::weak_ptr`: 用于管理共享对象，可以复制和移动

## 语法

需要引入 `memory` 头文件

```cpp
#include <memory>
```

### `std::unique_ptr`

```cpp
std::unique_ptr<类型> 指针名(new 类型(参数));
```

- 由于 `std::unique_ptr` 不能拷贝，只能移动；推荐使用 `std::make_unique` 而非裸 `new`

```cpp
std::unique_ptr<int> ptr1 = std::make_unique<int>(10);
```

### `std::shared_ptr`

```cpp
// 初始化内存
std::shared_ptr<类型> 指针名(new 类型(参数));
std::shared_ptr<类型> 指针名 = std::make_shared<类型>(参数);

// 只创建一个指向已有对象的智能指针
std::shared_ptr<类型> 指针名 = 一个共享智能指针;
```

- `std::shared_ptr` 类似于 `GLib` 中的 [`GObject`](https://github.com/EricLin0509/C/tree/main/应用API/GTK4/7.自定义GObject)
    - 只有首次创建时会分配内存
        - 对应 `GObject` 中的 `constructed()` 或 `init()` 方法 (`class_init` 是类级别初始化，每个类只执行一次)
    - 通过一个引用计数器来记录有多少个 `std::shared_ptr` 指向同一个对象
    - 当引用计数器为0时，对象会被自动释放
        - 对应 `GObject` 中的 `dispose` 和 `finalize` 方法

### `std::weak_ptr`

```cpp
std::weak_ptr<类型> 指针名 = 一个共享智能指针;
```

- `std::weak_ptr` 不会增加对象的引用计数器
    - 只能从 `std::shared_ptr` 或另一个 `std::weak_ptr` 构造
    - 当 `std::weak_ptr` 被销毁时，对象不会被自动释放
    - 通常用在只检查对象是否存在的场景

## 示例

现在有个类 `Object`

```cpp
class Object {
    public:
        Object()
        {
            std::cout << "Object created\n";
        }

        ~Object()
        {
            std::cout << "Object destroyed\n";
        }

        void print()
        {
            std::cout << "Hello World from Object\n";
        }
};
```

用这个类分别创建 `std::unique_ptr`、`std::shared_ptr` 和 `std::weak_ptr`

### `std::unique_ptr`

```cpp
std::unique_ptr<Object> ptr1 = std::make_unique<Object>();
```

此时可以看到输出

```
Object created
Hello World from Object
Object destroyed
```

这是因为当 `ptr1` 不再使用时，`std::unique_ptr` 会自动释放对象

现在我们想把 `ptr1` 赋值给 `ptr2`

```cpp
std::unique_ptr<Object> ptr2 = ptr1;
```

此时会报错

```
smart_pointer.cpp:26:36: error: 使用了被删除的函数‘std::unique_ptr<_Tp, _Dp>::unique_ptr(const std::unique_ptr<_Tp, _Dp>&) [with _Tp = Object; _Dp = std::default_delete<Object>]’
```

这是因为 `std::unique_ptr` 不允许复制，只能移动

这里有两种方法可以解决这个问题

一种是使用 `std::move` 函数

```cpp
std::unique_ptr<Object> ptr2 = std::move(ptr1);
```

但这样会导致 `ptr1` 失效 (所有权转移至 `ptr2`)

如果想让这两个指针都有效，就需要使用 `std::shared_ptr`

### `std::shared_ptr`

```cpp
std::shared_ptr<Object> ptr2 = std::make_shared<Object>();
std::shared_ptr<Object> ptr3 = ptr2;
ptr3->print();
```

此时只能看到一个 `Object destroyed` 的输出

```cpp
Object created
Hello World from Object
Object destroyed
```

这是因为 `ptr3` 只是跟 `ptr2` 指向同一个对象且增加了引用计数器，并没有分配新的内存

#### 查看引用计数

通过 `std::shared_ptr` 的 `use_count` 方法可以查看引用计数

```cpp
std::cout << "use_count: " << ptr2.use_count() << "\n";
```

此时输出

```
use_count: 2
```

- 2 表示 `ptr2` 和 `ptr3` 都指向同一个对象

### `std::weak_ptr`

`std::weak_ptr` 类似 `std::shared_ptr`，但不能保证对象是否还存在

```cpp
std::weak_ptr<Object> ptr4 = ptr2;
std::cout << "use_count: " << ptr2.use_count() << "\n";
```

此时输出

```
use_count: 2
```

因为 `ptr4` 是 `std::weak_ptr`，它不会增加对象的引用计数器

#### 解引用 `std::weak_ptr`

由于 `std::weak_ptr` 不保证对象是否还存在，所以不能直接解引用

```cpp
// ptr4->print(); // 错误
```

需要使用 `std::weak_ptr` 的 `lock` 方法来获取 `std::shared_ptr`

```cpp
std::shared_ptr<Object> shared = ptr4.lock();
std::cout << "use_count: " << ptr2.use_count() << "\n";
if (shared)
    shared->print();
```

此时输出

```
use_count: 3
Hello World from Object
```

由于 `shared` 是 `std::shared_ptr`，它会增加对象的引用计数器

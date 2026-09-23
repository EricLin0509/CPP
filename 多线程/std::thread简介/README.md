# std::thread简介

`std::thread` 是C++标准库提供的线程类，用于创建和管理线程

## 线程与进程的区别

- 线程是**进程中的一个执行流**，它共享进程的堆、全局数据等资源，但每个线程有独立的栈
- 进程是**独立的执行单位**，每个进程都有自己的地址空间和资源

## 示例

首先引入 `thread` 头文件

```cpp
#include <thread>
```

### 获取当前线程ID

使用 `std::this_thread::get_id` 获取当前线程ID

```cpp
std::cout << "Thread ID: " << std::this_thread::get_id() << "\n";
```

- **注意**: 线程ID与进程ID不同，线程ID是由标准库决定的，进程ID是由操作系统决定的

### 获取CPU核心数

使用 `std::thread::hardware_concurrency` 获取CPU核心数

```cpp
std::cout << "CPU cores: " << std::thread::hardware_concurrency() << "\n";
```

- 这个函数通常用于创建线程池的线程数量

### 创建线程

使用 `std::thread` 创建线程。一个 `std::thread` 对象只能表示一个线程

```cpp
std::thread t1();
```

此时会发出警告

```bash
threads.cpp:8:19: warning: empty parentheses were disambiguated as a function declaration [-Wvexing-parse]
    8 |     std::thread t1();
      |                   ^~
```

这是因为 `std::thread` 对象需要一个函数作为参数，用于创建线程

线程函数的可选签名形式如下（只需实现其中一种即可）

```cpp
void func_no_args(); // 示例1: 无参数
void func_by_value(int n); // 示例2: 按值传递
void func_by_ref(int& n); // 示例3: 按引用传递
void func_by_rvalue(int&& n); // 示例4: 按右值引用传递
```

- 每个线程对象只需选择其中一种签名实现即可
- 也可以使用 Lambda 表达式或类成员方法作为线程函数

### 实现线程函数

```cpp
void thread_func()
{
    std::cout << "Hello from thread " << std::this_thread::get_id() << "\n";
}
```

然后把线程函数作为参数传递给 `std::thread` 对象

```cpp
std::thread t1(thread_func);
```

但此时运行会发生错误

```bash
terminate called without an active exception
```

通过 `gdb` 查看具体信号

```bash
[New Thread 0x7ffff77ff6c0 (LWP 18283)]
[Thread 0x7ffff77ff6c0 (LWP 18283) exited]

Thread 1 "threads" received signal SIGABRT, Aborted.
0x00007ffff7899bfc in ?? () from /usr/lib/libc.so.6
```

这是因为线程对象在析构时仍然处于 **joinable** 状态。`std::thread` 的析构函数会检查：如果线程仍可 join，就调用 `std::terminate()` 终止程序

### 什么是 joinable 状态

`joinable` 状态是指线程对象是否关联了一个尚未同步的执行线程。可以通过 `t.joinable()` 查询

#### 为什么需要 joinable 状态

核心原因：**防止线程资源泄漏和程序异常终止**

1. **析构安全**：C++ 标准规定，如果一个 `std::thread` 对象在析构时仍处于 joinable 状态，就会调用 `std::terminate()` 终止整个程序。这是一种**强制性的安全检查**，防止开发者忘记同步线程

2. **资源不可回收**：joinable 的线程代表一个正在运行的执行流，操作系统为其分配了栈、寄存器状态、线程本地存储等资源。如果对象析构但线程仍在运行：
   - 如果自动 join：可能导致意外的阻塞，且隐藏了同步点，容易引发死锁
   - 如果自动 detach：线程可能访问已销毁的变量，导致未定义行为
   - 因此 C++ 选择了最安全的策略：**直接终止程序**，强制开发者显式处理

3. **设计争议**：`std::thread` 析构时调用 `std::terminate()` 实际上**不符合 RAII 原则**（RAII 要求析构函数释放资源而非终止程序）。这是 C++ 标准委员会的已知争议——没有一种隐式析构行为是安全的，因此只能要求开发者显式调用 `join()` 或 `detach()`

### join 与 detach

有两种方式让线程对象不再 joinable：

#### join

`join()` 会**阻塞当前线程**，等待子线程执行完毕

```cpp
std::thread t1(thread_func);
t1.join();
```

调用 `join()` 后，线程对象不再 joinable，析构时不会调用 `std::terminate()`

#### detach

`detach()` 会将子线程与线程对象**分离**，子线程在后台独立运行（守护线程）

```cpp
std::thread t1(thread_func);
t1.detach();
```

调用 `detach()` 后，线程对象不再 joinable，但子线程仍在运行，其生命周期由运行时管理

- **注意**: detach 后不能再对该线程调用 `join()`，也无法再获取该线程的执行结果

#### joinable

`joinable()` 用于判断线程对象是否可以 join 或 detach

```cpp
std::thread t1(thread_func);
std::cout << "joinable: " << t1.joinable() << "\n"; // 1 (true)
t1.join();
std::cout << "joinable: " << t1.joinable() << "\n"; // 0 (false)
```

为了防止多次 `join()` 或 `detach()`，建议在调用前检查 `joinable()`

```cpp
if (t1.joinable())
    t1.join();
```

### 检查线程是否有效

推荐使用 `joinable()` 判断线程对象是否关联了有效的执行线程

```cpp
if (!t1.joinable())
    std::cout << "Thread is not valid or already joined/detached\n";
```

也可以通过比较线程 ID 来判断，默认构造的线程对象其 ID 等于 `std::thread::id()`

```cpp
if (std::thread::id() == t1.get_id())
    std::cout << "Invalid thread ID\n";
```

- **注意**: `joinable()` 是惯用方式，ID 比较主要用于需要区分"默认构造"和"已 join/detach"的场景

### 向线程传递参数

直接在 `std::thread` 构造函数中追加参数即可

这里我们以按值传递为例

```cpp
void thread_func(int n)
{
    std::cout << "n = " << n << "\n";
}

std::thread t1(thread_func, 42);
t1.join();
```

### 线程休眠

使用 `std::this_thread::sleep_for` 让当前线程休眠

```cpp
#include <chrono>

std::this_thread::sleep_for(std::chrono::seconds(1));
std::this_thread::sleep_for(std::chrono::milliseconds(500));
```

### 总结

| 操作 | 说明 |
| :--: | :--: |
| `std::thread t(func)` | 创建线程并立即启动 |
| `t.join()` | 阻塞等待线程结束 |
| `t.detach()` | 分离线程，后台运行 |
| `t.joinable()` | 判断线程是否可 join/detach |
| `t.get_id()` | 获取指定线程对象的 ID |
| `std::this_thread::get_id()` | 获取当前线程 ID |
| `std::thread::hardware_concurrency()` | 获取 CPU 核心数 |
| `std::this_thread::sleep_for()` | 线程休眠 |

**核心原则**: `std::thread` 对象析构前必须调用 `join()` 或 `detach()`，否则程序会调用 `std::terminate()` 终止

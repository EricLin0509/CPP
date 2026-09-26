# 互斥锁

互斥锁 (`std::mutex`) 是 C++ 标准库提供的一个同步原语，用于保护共享资源的访问，确保同一时刻只有一个线程能进入临界区

## 示例

假设现在有个全局变量 `count`，用于多个线程同时计数1000000次

```cpp
static size_t count = 0;

void increment_count()
{
    for (size_t i = 0; i < 1000000; i++)
    {
        count++;
    }
}
```

尝试在两个以上线程中同时调用 `increment_count` 函数并输出最终的 `count`

```cpp
std::thread threads[2];

for (auto& thread : threads)
{
    thread = std::thread(increment_count);
}

for (auto& thread : threads)
{
    thread.join();
}

std::cout << "count: " << count << "\n";
```

预期的输出为应为 `2000000`

但实际输出为

```bash
count: 1215332
```

而且每次运行结果都不相同

```bash
count: 1593601
```

### 原因

#### 程序是怎么增加变量的值

1. 从内存中读取变量的值
2. 增加变量的值
3. 将变量的值写回内存

如果是单线程，那么这三个步骤是顺序执行的，不会出现竟态条件

但是如果是多线程，那么这三个步骤可能会被打断，导致变量的值不正确

#### 图表示意图

##### 正常情况

假设初始值为25

|        | thread1 | thread2 |
| :---: | :---: | :---: |
| 读取 | 25 | 26 |
| 增加 | 26 | 27 |
| 写回 | 26 | 27 |

结果：27

##### 假设线程2被打断

假设初始值为25，且已经循环了4次

|        | thread1 | thread2 |
| :---: | :---: | :---: |
| 读取 | 29 | 25 |
| 增加 | 30 | 26 |
| 写回 | 30 | 26 |

结果：26

这里 `thread2` 在 `thread1` 写回之前就读取了旧值 25，属于典型的**读-写竞态**，导致 `thread1` 的写回结果被 `thread2` 覆盖

### 解决方案——互斥锁

我们可以使用互斥锁来保护 `count` 变量的访问

需要引入 `mutex` 头文件

```cpp
#include <mutex>
```

#### 设置全局互斥锁变量

```cpp
static std::mutex count_mutex;
```

#### 加锁与解锁

```cpp
void increment_count()
{
    for (size_t i = 0; i < 1000000; i++)
    {
        count_mutex.lock();
        count++;
        count_mutex.unlock();
    }
}
```

- **注意**：每次加锁并完成操作后，必须解锁，否则会导致死锁

此时，`count` 的值为 `2000000`

```bash
count: 2000000
```

### RAII 锁管理器

RAII (Resource Acquisition Is Initialization) 是一种在 C++ 语言中用于管理资源的机制，确保在资源使用完成后能够及时释放

可以利用 RAII 来管理互斥锁，在资源使用完成后自动解锁，避免死锁

有两种 RAII 锁管理器

- `std::lock_guard`: 用于在代码块中自动加锁和解锁互斥锁，轻量级，不可手动解锁
- `std::unique_lock`: 比 `lock_guard` 更灵活，支持延迟加锁、手动解锁、与条件变量配合使用等

#### std::lock_guard

`std::lock_guard` 是一个模板类，用于在代码块中自动加锁和解锁互斥锁

```cpp
void increment_count()
{
    for (size_t i = 0; i < 1000000; i++)
    {
        std::lock_guard<std::mutex> lock(count_mutex);
        count++;
    }
}
```

#### std::unique_lock

`std::unique_lock` 是一个模板类，比 `lock_guard` 更灵活，支持延迟加锁、手动解锁等操作

```cpp
void increment_count()
{
    for (size_t i = 0; i < 1000000; i++)
    {
        std::unique_lock<std::mutex> lock(count_mutex);
        count++;
    }
}
```

也可以使用 `std::defer_lock` 延迟加锁，在需要时再手动加锁

```cpp
std::unique_lock<std::mutex> lock(count_mutex, std::defer_lock);
std::cout << "Now lock!\n"; // 不需要加锁
lock.lock();
count++;
```

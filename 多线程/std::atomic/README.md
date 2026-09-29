# std::atomic

`std::atomic` 是一个原子类型，用于在多线程环境中安全地操作整数类型

相比 `std::mutex`，`std::atomic` 用于单独同步操作一个变量，而 `std::mutex` 可以用于同步操作多个变量

## 语法

需要引入 `atomic` 头文件

```cpp
#include <atomic>
```

```cpp
std::atomic<类型> 变量名;
std::atomic<类型> 变量名 = 初始值;
```

## 常用操作

### load

原子地读取值

```cpp
std::atomic<int> a = 10;
int val = a.load(); // val = 10
```

### store

原子地写入值

```cpp
std::atomic<int> a = 10;
a.store(20); // a = 20
```

### wait / notify_one / notify_all（C++20）

此概念上与条件变量类似 (等待-通知模型), 但无需 `mutex` 且更轻量

`wait` 阻塞当前线程，直到值发生变化且被通知；`notify_one` 唤醒一个等待线程，`notify_all` 唤醒所有等待线程

```cpp
std::atomic<int> flag = 0;

// 线程 A：等待 flag 变为 1
flag.wait(0); // 阻塞，直到 flag != 0 且有人调用 notify

// 线程 B：修改并通知
flag.store(1);
flag.notify_one(); // 唤醒一个在 flag 上等待的线程
```

- `wait(old)` 的语义：如果当前值等于 `old`，则阻塞；否则立即返回。被唤醒后会重新检查值

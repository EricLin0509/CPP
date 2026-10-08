# thread_local

`thread_local` 是 C++11 引入的线程本地存储变量，用于在每个线程中维护一个独立的变量实例。

## 基本概念

- 每个线程拥有该变量的独立副本，线程之间互不干扰
- 生命周期与线程一致：线程启动时构造，线程结束时析构
- 与 `static` 和 `extern` 可组合使用，但 `thread_local` 的存储期优先

## 语法

```cpp
thread_local 类型 变量名 = 初始值;
thread_local static 类型 变量名 = 初始值;
thread_local extern 类型 变量名 = 初始值;
```

- 每个线程都只初始化一次，后续访问直接使用已初始化的副本

## 示例

假设现在有个函数 `calculate`

```cpp
int calculate(void)
{
    std::cout << "The function calculate() is called\n";
    return 42; // 假设计算结果为 42
}
```

我们希望在每个线程中调用 `calculate` 函数

```cpp
void thread_func(void)
{
    for (int i = 0; i < 10; ++i)
    {
        int result = calculate();
    }
}
```

```cpp
std::thread t1(thread_func);
std::thread t2(thread_func);
```

此时通过输出

```bash
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
The function calculate() is called
```

可以看到，每个线程都调用了 `calculate` 函数 10 次，所以总共有 20 次调用

但这样在高并发场景中，每次都要重新计算同样的结果42，这是非常浪费资源的

那有什么更好的方法来避免重复计算 42 呢？

### 使用 thread_local 变量

采用 `thread_local` 变量 `result` 来存储计算结果 42，后续访问直接使用已初始化的副本

```cpp
void thread_func(void)
{
    for (int i = 0; i < 10; ++i)
    {
        thread_local int result = calculate();
    }
}
```

此时通过输出

```bash
The function calculate() is called
The function calculate() is called
```

可以看到，每个线程只调用了 1 次 `calculate` 函数，所以总共有 2 次调用

从20次调用变成了2次调用，大大提高了效率

#### 为什么循环内的 `thread_local` 不会每次迭代都重新初始化?

`thread_local` 是**存储期说明符**，不是普通变量声明。它的语义是：每个线程首次执行到该语句时初始化一次，之后该线程内所有对该变量的访问都复用已初始化的副本。因此即使写在循环体内，也不会每次迭代都重新调用 `calculate()`

这正是 `thread_local` 的核心价值：**在需要的位置声明，享受线程级缓存的效率**

## 与 static 的区别

如果把 `thread_local` 换成 `static`：

```cpp
void thread_func(void)
{
    for (int i = 0; i < 10; ++i)
    {
        static int result = calculate();  // 整个程序只初始化一次
    }
}
```

此时输出只有 **1 次**：

```bash
The function calculate() is called
```

因为 `static` 是**进程级单例**，所有线程共享同一个 `result`，`calculate()` 只在首次执行时调用一次。

而 `thread_local` 是**线程级单例**，每个线程各自拥有独立的 `result`，所以 2 个线程各调用一次，共 2 次。

| | `static` | `thread_local` |
| :---: | :---: | :---: |
| 实例数量 | 整个程序 1 个 | 每个线程 1 个 |
| `calculate()` 调用次数 | 1 次 | N 次（N = 线程数） |
| 多线程写入 | ⚠️ 不安全，需加锁 | ✅ 天然安全，各线程独立 |
| 生命周期 | 程序启动 → 结束 | 线程启动 → 结束 |
| 适用场景 | 全局共享的常量/缓存 | 线程独立的缓存/状态 |

- **选择原则**：如果结果对所有线程都相同且只读，用 `static` 更省；如果每个线程需要独立的状态或缓存，用 `thread_local` 避免锁竞争。
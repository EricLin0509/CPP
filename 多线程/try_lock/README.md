# try_lock

`try_lock()` 方法尝试加锁但不阻塞，返回值为 `bool`，表示是否成功加锁

- 如果互斥锁未被占用，加锁成功，返回 `true`
- 如果互斥锁已被其他线程占用，加锁失败，返回 `false`

## 与 lock() 方法的区别

| 特征 | `try_lock()` | `lock()` |
| :--: | :---------: | :------: |
| 加锁是否阻塞 | 否 | 是 |
| 返回值 | `bool` | 无返回值 |
| 加锁失败处理 | 返回 `false`，继续执行 | 阻塞等待直到加锁成功 |
| 适用场景 | 非阻塞检查、遍历多个资源 | 需要阻塞等待 |

## 示例

假设有 4 个炉子，每个炉子有 100 单位燃料，10 个线程竞争使用炉子

每个线程需要随机数量的燃料，使用 `try_lock()` 非阻塞地尝试获取炉子——哪个空用哪个

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <random>

static constexpr int STOVE_COUNT = 4;
static constexpr int THREAD_COUNT = 10;

static std::mutex stove_mutexes[STOVE_COUNT];
static int stove_fuel[STOVE_COUNT] = {100, 100, 100, 100};
```

### 线程函数

遍历所有炉子，用 `try_lock()` 非阻塞地尝试获取

- 加锁成功 → 使用炉子，消耗燃料，解锁退出
- 加锁失败 → 尝试下一个炉子
- 所有炉子都忙 → 等待后重试

```cpp
void routine()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 30);

    for (int i = 0; i < STOVE_COUNT; i++)
    {
        if (stove_mutexes[i].try_lock()) // 尝试使用当前炉子
        {
            int needed_fuel = dist(gen);

            if (stove_fuel[i] - needed_fuel < 0) // 燃料不足
            {
                std::cout << "炉子 " << i << " 燃料不足\n";
            }
            else // 燃料足够
            {
                stove_fuel[i] -= needed_fuel;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                std::cout << "炉子 " << i << " 消耗 " << needed_fuel
                          << "，剩余 " << stove_fuel[i] << "\n";
            }

            stove_mutexes[i].unlock();
            return;
        }
        else
        {
            if (i == STOVE_COUNT - 1) // 所有炉子都忙
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(300)); // 等待 300ms 后重试
                i = -1;
            }
        }
    }
}
```

- `i = -1` 而非 `i = 0`，因为 `for` 循环的 `i++` 会执行，下一轮 `i` 从 0 开始

### 运行

```cpp
int main()
{
    std::thread threads[THREAD_COUNT];

    for (auto& t : threads)
    {
        t = std::thread(routine);
    }

    for (auto& t : threads)
    {
        t.join();
    }

    return 0;
}
```

输出大致如下

```bash
炉子 0 消耗 22，剩余 78
炉子 1 消耗 18，剩余 82
炉子 2 消耗 21，剩余 79
炉子 3 消耗 23，剩余 77
炉子 0 消耗 23，剩余 55
炉子 1 消耗 28，剩余 54
炉子 2 消耗 15，剩余 64
炉子 3 消耗 3，剩余 74
炉子 0 消耗 27，剩余 28
炉子 1 消耗 28，剩余 26
```

可以看到，`try_lock()` 让线程不会阻塞在某个炉子上等待，而是**非阻塞地尝试下一个炉子**，实现了"哪个空用哪个"的调度策略

# 条件变量

条件变量 (condition variable) 是一种同步机制，用于在多个线程之间进行通信和协调

需要配合互斥锁 (mutex) 使用，以确保线程安全

## 定义

条件变量是利用线程间共享的全局变量进行同步的一种机制，主要包括两个动作：一个线程等待"条件变量的条件成立"而挂起；另一个线程使"条件成立"（给出条件成立信号）

## 情景

假设现在有一辆汽车，现在有两个线程，一个线程是加油的线程，一个线程是等待加油的线程

- `fuel_filling`: 加油
- `car`: 车

### 定义两个函数

```cpp
std::mutex mtx;
int fuel = 0;

void fuel_filling()
{
    for (int i = 0; i < 5; i++)
    {
        std::lock_guard<std::mutex> lock(mtx);
        fuel += 15;
        std::cout << "加油中... " << fuel << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void car()
{
    std::lock_guard<std::mutex> lock(mtx);
    fuel -= 40;
    std::cout << "请立即加油! 现在剩余：" << fuel << std::endl;
}
```

这两个函数的作用分别是加油和等待加油

### 创建两个线程分别调用这两个函数

```cpp
std::mutex mtx;
int fuel = 0;

int main() {
    std::thread t1(fuel_filling);
    std::thread t2(car);

    t1.join();
    t2.join();

    return 0;
}
```

### 运行

```bash
g++ -o conditionVariable conditionVariable.cpp -pthread
./conditionVariable
请立即加油! 现在剩余：-40
加油中... -25
加油中... -10
加油中... 5
加油中... 20
加油中... 35
```

可以看到，油量出现了负值

### 尝试解决

```cpp
void car()
{
    std::unique_lock<std::mutex> lock(mtx);
    while (fuel < 40)
    {
        std::cout << "没油了，等待中..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    fuel -= 40;
    std::cout << "请立即加油! 现在剩余：" << fuel << std::endl;
}
```

### 再次运行

```bash
g++ -o conditionVariable conditionVariable.cpp -pthread
./conditionVariable
没油了，等待中...
没油了，等待中...
没油了，等待中...
...
```

可以看到，程序一直在等待中，没有任何输出，因为 `mutex` 一直被占用着

## 解决方案——std::condition_variable

`std::condition_variable` 主要有以下方法：
- `wait`: 等待条件变量
- `notify_all`: 广播条件变量（唤醒所有等待线程）
- `notify_one`: 发送条件变量（唤醒一个等待线程）

### 定义条件变量

```cpp
std::condition_variable cv;
```

### 使用条件变量

#### 等待条件变量

```cpp
std::mutex mtx;
int fuel = 0;
std::condition_variable cv;

void fuel_filling()
{
    for (int i = 0; i < 5; i++)
    {
        std::lock_guard<std::mutex> lock(mtx);
        fuel += 15;
        std::cout << "加油中... " << fuel << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void car()
{
    std::unique_lock<std::mutex> lock(mtx);
    while (fuel < 40)
    {
        std::cout << "没油了，等待中..." << std::endl;
        cv.wait(lock); // 等待条件变量
    }
    fuel -= 40;
    std::cout << "请立即加油! 现在剩余：" << fuel << std::endl;
}
```

**注意：当执行 `cv.wait(lock);` 时，会先解锁 `mtx`，然后等待条件变量，当条件变量被通知时，会先加锁 `mtx`，然后执行后面的代码**

#### 发送条件变量

```cpp
std::mutex mtx;
int fuel = 0;
std::condition_variable cv;

void fuel_filling()
{
    for (int i = 0; i < 5; i++)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            fuel += 15;
            std::cout << "加油中... " << fuel << std::endl;
        }
        cv.notify_one(); // 发送条件变量
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void car()
{
    std::unique_lock<std::mutex> lock(mtx);
    while (fuel < 40)
    {
        std::cout << "没油了，等待中..." << std::endl;
        cv.wait(lock); // 等待条件变量
        // 等价于：
        // lock.unlock();
        // 等待条件变量
        // lock.lock();
    }
    fuel -= 40;
    std::cout << "请立即加油! 现在剩余：" << fuel << std::endl;
}
```

### 运行

```bash
g++ -o conditionVariable conditionVariable.cpp -pthread
./conditionVariable
没油了，等待中...
加油中... 15
没油了，等待中...
加油中... 30
没油了，等待中...
加油中... 45
请立即加油! 现在剩余：5
加油中... 20
加油中... 35
```

可以看到，程序正常运行了

## C++ 与 C 条件变量对比

| C (pthread) | C++ (std) |
|---|---|
| `pthread_mutex_t` | `std::mutex` |
| `pthread_cond_t` | `std::condition_variable` |
| `pthread_cond_wait(&cond, &mutex)` | `cv.wait(lock)` |
| `pthread_cond_signal(&cond)` | `cv.notify_one()` |
| `pthread_cond_broadcast(&cond)` | `cv.notify_all()` |
| `pthread_mutex_lock(&mutex)` | `std::lock_guard` / `std::unique_lock` |
| `pthread_mutex_unlock(&mutex)` | 自动解锁（RAII） |

## 总结

条件变量相当于一个控制信号的标识符，可以发送信号，也可以等待信号，当信号被发送时，会唤醒等待的线程，然后执行后面的代码

C++ 的条件变量相比 C 的 pthread 版本，利用 RAII 机制自动管理锁的生命周期，避免了手动加锁/解锁带来的遗漏风险

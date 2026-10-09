# 配合shared_ptr

在多线程环境下，配合shared_ptr使用以提高性能

## 示例

假设现在有个结构体 `Stock`

```cpp
struct Stock {
    std::string id;
    double price;
    double sell;
    double bid;
    int date;
    int time;

    Stock(std::string id_ = {}, double price_ = {}, double sell_ = {},
          double bid_ = {}, int date_ = {}, int time_ = {})
        : id(std::move(id_)), price(price_), sell(sell_),
          bid(bid_), date(date_), time(time_) {}
};
```

基于这个结构体，我们可以定义一个 `StockRCU` 类

```cpp
class StockRCU {
    private:
        Stock stock;
        mutable std::shared_mutex read_mutex;
    public:
        StockRCU();
        ~StockRCU();

        Stock read() const
        {
            std::shared_lock lock(read_mutex);
            return stock;
        }

        void update(std::string id, double price,
                    double sell, double bid, 
                    int date, int time)
        {
            std::unique_lock lock(read_mutex);
            stock.id = id;
            stock.price = price;
            stock.sell = sell;
            stock.bid = bid;
            stock.date = date;
            stock.time = time;
        }
};
```

- **为什么 `read()` 返回拷贝而不是引用？**
    - 返回 `Stock` 拷贝可以让读者拿到一份**快照**，之后对数据的修改不会影响已读取的快照，从而避免数据竞争。如果返回引用，读者在使用期间数据可能被写者修改，导致读取到不一致的状态

- **为什么 `read_mutex` 需要 `mutable`？**
    - `read()` 是 `const` 成员函数，但加锁操作会修改 mutex 的内部状态，因此必须将 `read_mutex` 声明为 `mutable` 才能在 `const` 方法中使用

但是这里有个问题，在执行 `std::unique_lock` 后，它需要更新6个字段。但如果是更新100个字段会导致锁粒度太大，影响性能

那我们该如何优化呢？

### 采用 `shared_ptr`

我们可以使用 `shared_ptr` 中的 `swap` 方法来更新 `stock` 结构体，而不是直接更新字段

```cpp
class StockRCU {
    private:
        std::shared_ptr<Stock> stock;
        mutable std::shared_mutex read_mutex;
    public:
        StockRCU() : stock(std::make_shared<Stock>()) {}

        std::shared_ptr<Stock> read() const
        {
            std::shared_lock lock(read_mutex);
            return stock;
        }

        void update(std::string id, double price,
                    double sell, double bid, 
                    int date, int time)
        {
            std::shared_ptr<Stock> new_stock = std::make_shared<Stock>(id, price, sell, bid, date, time);
            std::unique_lock lock(read_mutex);
            stock.swap(new_stock);
        }
};
```

这样在 `update` 方法中，只需要更新1次，从而提高性能

### 原理：RCU（Read-Copy-Update）

这种模式的核心思想是：

1. **Read**：读者通过 `shared_ptr` 获取当前数据的引用，引用计数 +1
2. **Copy**：写者先构造一份完整的新数据（在锁外完成大部分工作）
3. **Update**：写者通过 `swap` 原子地替换指针，旧数据仍被读者持有的 `shared_ptr` 引用

关键的安全性保证：
- 读者持有的 `shared_ptr` 使旧数据的引用计数不为零，旧数据不会被销毁
- 当读者释放 `shared_ptr` 后，引用计数归零，旧数据自动销毁
- 写者只修改指针本身，不修改数据内容，因此读者永远不会看到不一致的状态

### ⚠️ `shared_ptr` 的线程安全性

这是一个常见的误区：

- `shared_ptr` 的**控制块**（引用计数）是线程安全的，多个线程可以安全地拷贝同一个 `shared_ptr`
- 但 `shared_ptr` **本身的读写**（赋值、swap、reset）**不是线程安全的**，多个线程同时修改同一个 `shared_ptr` 变量会导致数据竞争

因此，我们仍然需要 `shared_mutex` 来保护 `stock` 的 `swap` 操作。锁的作用不是保护 `Stock` 数据，而是保护 `shared_ptr` 指针本身的替换

### 进一步优化：C++20 `std::atomic<std::shared_ptr>`

C++20 引入了 `std::atomic<std::shared_ptr<T>>`，可以完全去掉 `shared_mutex`：

```cpp
class StockRCU {
    private:
        std::atomic<std::shared_ptr<Stock>> stock;
    public:
        StockRCU() : stock(std::make_shared<Stock>()) {}

        std::shared_ptr<Stock> read() const
        {
            return stock.load();
        }

        void update(std::string id, double price,
                    double sell, double bid,
                    int date, int time)
        {
            auto new_stock = std::make_shared<Stock>(id, price, sell, bid, date, time);
            stock.store(new_stock);
        }
};
```

这样连锁都不需要了，读者和写者完全无锁并发，性能最优

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <atomic>

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

int main() {
    StockRCU stock_rcu;
    std::thread p1([&stock_rcu]() {
        stock_rcu.update("AAPL", 100.0, 100.0, 100.0, 2023, 1);
    });
    std::thread p2([&stock_rcu]() {
        auto stock = stock_rcu.read();
        std::cout << stock->id << "\n";
        std::cout << stock->price << "\n";
        std::cout << stock->sell << "\n";
        std::cout << stock->bid << "\n";
        std::cout << stock->date << "\n";
        std::cout << stock->time << "\n";
    });
    p1.join();
    p2.join();

    return 0;
}
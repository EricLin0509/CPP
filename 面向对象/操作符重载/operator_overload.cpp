#include <iostream>

class Point {
    private:
        int data;

    public:
        Point()
        {
            this->data = 0;
        }

        Point(int data)
        {
            this->data = data;
        }

        int getData() const
        {
            return this->data;
        }

        int operator+(const Point& other) const // 重载 `+` 操作符
        {
            return this->data + other.getData(); // 返回两个 Point 对象的和
        }

        Point operator+(int other) const // 重载 `+` 操作符
        {
            return Point(this->data + other); // 返回一个 Point 对象
        }

        bool operator==(const Point& other) const // 重载 `==` 操作符
        {
            return this->data == other.getData(); // 比较两个 Point 对象的值
        }
};

int main() {
    Point p1(10);
    Point p2(20);
    std::cout << "Sum: " << p1 + p2 << "\n";
    
    Point p3 = p1 + 30;

    std::cout << "p3: " << p3.getData() << "\n";

    bool isEqual = p1 == p2;

    std::cout << "p1 == p2: " << (isEqual ? "true" : "false") << "\n";

    return 0;
}

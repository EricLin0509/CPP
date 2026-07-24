# 操作符重载

在C++中，操作符重载允许在类中定义新的操作符，使得类对象可以像内置类型一样使用这些操作符

这在需要对类对象进行自定义操作时非常有用

## 语法

```cpp
返回值 operator<操作符>(参数列表)
{
    // 操作符重载的实现
}
```

- 操作符：需要重载的操作符，可以是运算操作符、关系操作符、位操作符、下标操作符等
    - 运算操作符：如 `+`, `-`, `*`, `/`
    - 关系操作符：如 `==`, `!=`, `<`, `>`, `<=`, `>=`
    - 位操作符：如 `&`, `|`, `^`, `<<`, `>>`
    - 下标操作符：`[]` (用于数组访问)


## 示例

假设现在有个 `Point` 类，用于存储一个整数

```cpp
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
};
```

创建两个 `Point` 对象

```cpp
Point p1(10);
Point p2(20);
```

现在，我们希望对这两个 `Point` 对象进行加法运算，但默认情况下，C++ 不支持 `Point` 类对象的加法运算

```cpp
std::cout << "Sum: " << p1 + p2 << "\n";
```

```
operator_overload.cpp:23:19: error: no match for ‘operator+’ (operand types are ‘Point’ and ‘Point’)
```

为了实现 `Point` 类对象的加法运算，我们需要重载 `+` 操作符

### 实现操作符重载

```cpp
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
};
```

现在，我们就可以对两个 `Point` 对象进行加法运算了

```cpp
Point p1(10);
Point p2(20);

std::cout << "Sum: " << p1 + p2 << "\n";
```

```
Sum: 30
```

### 函数重载支持

操作符重载也同样支持函数重载 (即可以设计多个**不同**的 `operator+` 函数)

```cpp
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
};
```

```cpp
std::cout << "Sum: " << p1 + p2 << "\n"; // 调用的是第一个 `operator+` 函数
Point p3 = p1 + 10; // 调用的是第二个 `operator+` 函数
```

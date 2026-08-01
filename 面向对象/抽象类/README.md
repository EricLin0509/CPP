# 抽象类

抽象类 (Abstract Class) 是C++中一种特殊的类，它不能被实例化，只能被继承

抽象类必须包含至少一个纯虚方法 (Pure Virtual Method)

## 语法

```cpp
class 抽象类名 {
    virtual 返回类型 方法名1() = 0;
    virtual 返回类型 方法名2() = 0;
    // ...
};
```

- `=0` 表示如果子类要实现实例化,则必须实现所有纯虚方法
- 不能创建抽象类的对象
- 抽象类可以作为基类被继承
- 纯虚方法可以有默认实现

## 示例

现在有个抽象类 `Shape`，它包含一个纯虚方法 `area()` 和 `perimeter()`，表示计算面积和周长

```cpp
class Shape {
    virtual double area() = 0;
    virtual double perimeter() = 0;
};
```

现在有一个子类 `Circle`，它继承自 `Shape`

```cpp
class Circle : public Shape {
    private:
        double radius;
    public:
        Circle(double r)
        {
            this->radius = r;
        }
};
```

此时尝试编译会报错

```
abstract_class.cpp:19:12: error: 不能将变量‘circle’声明为具有抽象类型‘Circle’
abstract_class.cpp:8:7: note: because the following virtual functions are pure within ‘Circle’
```

这是因为没有实现 `area()` 和 `perimeter()` 方法

### 实现纯虚方法

```cpp
class Circle : public Shape {
    private:
        double radius;
    public:
        Circle(double r)
        {
            this->radius = r;
        }
        double area() override
        {
            return 3.14 * this->radius * this->radius;
        }
        double perimeter() override
        {
            return 2 * 3.14 * this->radius;
        }
};
```

### 使用 Circle 类

```cpp
Circle circle(5);
std::cout << "Circle area: " << circle.area() << "\n";
std::cout << "Circle perimeter: " << circle.perimeter() << "\n";
```

```
Circle area: 78.5
Circle perimeter: 31.4
```

### 默认实现

可以在抽象类中为纯虚方法提供默认实现

- **注意**：提供默认实现的纯虚函数需要在子类可访问的作用域 (如 `protected` 或 `public`),否则子类无法调用默认实现

```cpp
class Shape {
    virtual double area() = 0;
    virtual double perimeter() = 0;
    protected:
        virtual void print() = 0;
};
```

这里为 `print()` 方法提供默认实现

```cpp
void Shape::print()
{
    std::cout << "[Warning] Shape::print() is not implemented!\n";
}
```

然后在需要**实例化的子类**中实现该 `print()` 方法

```cpp
class Circle : public Shape {
    private:
        double radius;
    public:
        Circle(double r)
        {
            this->radius = r;
        }
        double area() override
        {
            return 3.14 * this->radius * this->radius;
        }
        double perimeter() override
        {
            return 2 * 3.14 * this->radius;
        }
        void print() override // 调用默认实现同样需要重写
        {
            Shape::print();
        }
};
```

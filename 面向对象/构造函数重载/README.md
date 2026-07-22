# 构造函数重载

构造函数重载是指在同一个类中定义多个构造函数，每个构造函数的**参数列表不同**

构造函数重载的主要目的是为了满足不同的初始化需求

## 语法

```cpp
class 类名 {
    权限修饰符:
        构造函数名(参数列表1) {    }
        构造函数名(参数列表2) {    }
        ...
};
```

- 每个构造函数的**参数列表必须不同**

## 示例

假设现在有一个类 `Pizza`，它可以选两种配料

```cpp
class Pizza {
    public:
        std::string topping1;
        std::string topping2;
}
```

现在可以创建一个只包含一种配料的披萨

```cpp
class Pizza {
    public:
        std::string topping1;
        std::string topping2;

        Pizza(std::string topping1)
        {
            this->topping1 = topping1;
            this->topping2 = "";
        }
};
```

```cpp
Pizza pizza1("Pepperoni");
```

如果用户想创建一个包含两种配料的披萨

```cpp
Pizza pizza2("Pepperoni", "Mushrooms");
```

此时会出现编译错误，因为构造函数的参数列表不同

```
overload_constructor.cpp:17:42: error: 对‘Pizza::Pizza(const char [10], const char [10])’的调用没有匹配的函数
```

此时可以使用构造函数重载来解决这个问题

### 使用构造函数重载

```cpp
class Pizza {
    public:
        std::string topping1;
        std::string topping2;

        Pizza(std::string topping1)
        {
            this->topping1 = topping1;
            this->topping2 = "";
        }

        Pizza(std::string topping1, std::string topping2)
        {
            this->topping1 = topping1;
            this->topping2 = topping2;
        }
};
```

现在可以创建一个只包含一种配料的披萨

```cpp
Pizza pizza1("Pepperoni");
```

也可以创建一个包含两种配料的披萨

```cpp
Pizza pizza2("Pepperoni", "Mushrooms");
```

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
- 构造函数重载与普通函数重载的规则相同
    - 参数类型不同
    - 参数个数不同
    - 参数顺序不同

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

此时会出现编译错误，因为没有匹配两个参数的构造函数

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

### 默认构造函数

通常情况下，编译器会自动生成一个默认构造函数，该构造函数不带参数，且不执行任何操作

如果定义了任何带参数的构造函数，编译器将不再自动生成默认构造函数

此时如果需要无参构造，必须显式定义

```cpp
class Pizza {
    public:
        std::string topping1;
        std::string topping2;

        Pizza()
        {
            this->topping1 = "Pepperoni";
            this->topping2 = "";
        }
};
```

此时可以创建一个不指定配料的披萨

```cpp
Pizza pizza3; // 括号可以省略
```


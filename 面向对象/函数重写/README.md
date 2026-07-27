# 函数重写

函数重写 (Function Overriding) 是面向对象编程中的一种特性，它允许子类重写父类的方法

重写的方法必须具有相同的名称、参数列表和返回类型

重写的主要目的是为了实现多态性，即同一个接口可以有不同的实现

## 与函数重载的区别

- 作用域不同
    - 函数重载是在同一个类中定义的
    - 函数重写是在不同的类中定义的
- 参数列表不同
    - 函数重载可以参数个数、类型或顺序不同
    - 函数重写必须参数个数、类型和顺序相同
- 返回类型不同
    - 函数重载的返回类型可以相同也可以不同
    - 函数重写的返回类型必须相同或是协变类型
        - 协变类型：父类方法返回类型的子类的指针或引用

## 语法

使用 `virtual` 关键字声明父类的方法，允许在子类中重写

```cpp
class 父类 {
    public:
        virtual 返回类型 方法名(参数列表)
        {
            // 父类方法的实现
        }
};

class 子类 : public 父类 {
    public:
        // 重写父类的方法
        virtual 返回类型 方法名(参数列表)
        {
            // 子类方法的实现
        }
};
```

- 如果子类没有重写父类的方法，那么子类的方法会继承父类的方法 (即调用父类的实现)

### 使用 `override` 关键字

C++11 引入了 `override` 关键字，用于检查子类是否正确重写了父类的方法

```cpp
class 父类 {
    public:
        virtual 返回类型 方法名(参数列表)
        {
            // 父类方法的实现
        }
};

class 子类 : public 父类 {
    public:
        // 重写父类的方法
        返回类型 方法名(参数列表) override
        {
            // 子类方法的实现
        }
};
```

- 如果子类重写的方法的返回类型或参数列表与父类的方法不匹配，编译器会报错

## 示例

这里创建两个类 `firstName` 和 `lastName`，`firstName` 是 `lastName` 的父类

现在我们想要重写 `firstName` 类中的 `printName` 方法，使其输出 `First Name` 和 `Last Name`

要想重写 `printName` 方法，需要使用 `virtual` 关键字声明 `printName` 方法

```cpp
class firstName {
    protected:
        std::string first_name;
    public:
        firstName(std::string name)
        {
            this->first_name = name;
        }

        virtual void printName() const // 使用 virtual 关键字声明 printName 方法
        {
            std::cout << "First Name: " << first_name << "\n";
        }
};

class lastName : public firstName {
    private:
        std::string last_name;
    public:
        lastName(std::string first_name, std::string last_name) : firstName(first_name)
        {
            this->last_name = last_name;
        }

        virtual void printName() const
        {
            std::cout << "First Name: " << this->first_name << "\n";
            std::cout << "Last Name: " << this->last_name << "\n";
        }
};
```

### 创建对象并调用方法

```cpp
lastName person("John", "Doe");
person.printName();
```

此时会输出

```
First Name: John
Last Name: Doe
```

### 使用 `override` 关键字

使用 `override` 关键字可以检查子类是否正确重写了父类的方法

```cpp
class firstName {
    protected:
        std::string first_name;
    public:
        firstName(std::string name)
        {
            this->first_name = name;
        }

        virtual void printName() const
        {
            std::cout << "First Name: " << first_name << "\n";
        }
};

class lastName : public firstName {
    private:
        std::string last_name;
    public:
        lastName(std::string first_name, std::string last_name) : firstName(first_name)
        {
            this->last_name = last_name;
        }

        void printName() const override
        {
            std::cout << "First Name: " << this->first_name << "\n";
            std::cout << "Last Name: " << this->last_name << "\n";
        }
};
```

假设我们不小心将 `printName` 方法的参数列表写错了，编译器会报错

```cpp
void printName(int i = 0) const override
```

```
function_override.cpp:27:14: error: ‘void lastName::printName(int) const’ marked ‘override’, but does not override
```

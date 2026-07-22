# 继承

继承是面向对象编程中的一个重要概念

它允许一个类（子类）继承另一个类（父类）的属性和方法

继承的主要目的是代码复用和扩展

## 语法

```cpp
class 父类 {
    权限修饰符:
        成员变量;
        成员函数;
};

class 子类 : 继承方式 父类 {
    权限修饰符:
        成员变量;
        成员函数;
};
```

- 继承方式：`public`、`private`、`protected`
    - `public`：父类的公有成员在子类中保持公有，保护成员保持保护
    - `private`：父类的所有成员（公有、保护）在子类中都变为私有
    - `protected`：父类的公有成员和保护成员在子类中都变为保护

## 示例

现在有一个 `Animal` 类

```cpp
class Animal {
    public:
        bool isAlive = true;

        void eat() {
            std::cout << "This animal is eating" << "\n";
        }
};
```

现在有一个 `Dog` 类，继承自 `Animal` 类

```cpp
class Dog : public Animal {

};
```

虽说现在 `Dog` 类没有实现任何成员函数，但可以访问 `Animal` 类的公有成员变量和公有成员函数

```cpp
Dog dog;

std::cout << (dog.isAlive ? "true" : "false") << "\n"; // 输出：true
dog.eat(); // 输出：This animal is eating
```

### 设置子类专属成员

子类也可以设置专属成员

```cpp
class Dog : public Animal {
    public:
        void bark()
        {
            std::cout << "The dog goes woof!" << "\n";
        }
};
```

```cpp
dog.bark(); // 输出：The dog goes woof!
```

但这个 `bark` 方法只能在 `Dog` 类中使用，不能在父类或者未继承 `Dog` 类的其他类中使用

```cpp
class Cat : public Animal {
    public:
        void meow()
        {
            std::cout << "The cat goes meow!" << "\n";
        }
};
```

```cpp
Cat cat;
cat.bark(); // 编译错误
```

```
inheritance.cpp:38:9: error: ‘class Cat’ has no member named ‘bark’
```

# 面向对象简介

面向对象编程是一种编程范式，它将数据和操作数据的方法封装在一起，形成对象

面向对象编程的核心概念包括类、对象、继承、多态和封装

## 语法

### 类定义

```cpp
class 类名 {
    权限修饰符:
        类型 成员变量1;
        类型 成员变量2;
        ...

        返回值 成员方法1(参数列表) {    }
        返回值 成员方法2(参数列表) {    }
        ...
};
```

- 权限修饰符：有 `public`、`private`、`protected` 三种
    - 分别控制成员变量和成员方法的访问权限

### 访问对象成员

```cpp
// 访问对象成员变量
对象名.成员变量名;

// 调用对象成员方法
对象名.成员方法名(参数列表);
```

## 示例

创建一个 `human` 类

### 类定义

```cpp
class Human {
    public:
        std::string name;
        int age;

        void eat()
        {
            std::cout << "This person is eating.\n";
        }
        void drink()
        {
            std::cout << "This person is drinking.\n";
        }
        void sleep()
        {
            std::cout << "This person is sleeping.\n";
        }
};
```

### 初始化对象

```cpp
Human person1;
```

### 访问成员变量

使用成员访问运算符 `.` 访问成员变量

```cpp
person1.name = "John";
person1.age = 25;

std::cout << "Name: " << person1.name << "\n";
std::cout << "Age: " << person1.age << "\n";
```

### 调用成员方法

与访问成员变量类似，使用成员访问运算符`.` 调用成员方法

```cpp
person1.eat();
person1.drink();
person1.sleep();
```

# 权限修饰符

权限修饰符用于控制类成员的访问权限

- `public`：类成员在任何地方都可以访问
- `private`：类成员只能在类内部访问
- `protected`：类成员只能在类内部和派生类中访问
- 如果没有指定权限修饰符，那么默认是 `private` 的
- 允许同时存在 `public`、`private`、`protected` 三种权限修饰符的成员变量

## 示例

现在我们有个类

```cpp
class Student {
    std::string name;
    int age;
};
```

当尝试访问对象的成员时，会报错

```cpp
Student stu;
std::cout << stu.name << "\n";
```

```
access.cpp:10:22: error: ‘std::string Student::name’ is private within this context
```

这是因为 `name` 和 `age` 是 `private` 的，只能在类内部访问

### 解决方法

需要使用 `public` 权限修饰符来修饰成员变量

```cpp
class Student {
    public:
        std::string name;
        int age;
};
```

此后就可以在类外部访问成员变量了

```cpp
Student stu;
stu.name = "John";
stu.age = 20;
std::cout << stu.name << "\n";
```

### protected 修饰符

`protected` 修饰符与 `private` 修饰符类似，但允许派生类访问

现在我们有一个类 `Person`

```cpp
class Person {
    protected:
        std::string name = "John";
        int age = 20;
};
```

然后我们创建一个派生类 `Student`，并访问 `Person` 的 `name` 和 `age` 成员变量

```cpp
class Student : public Person {
    public:
        void display()
        {
            std::cout << "Name: " << name << "\n";
            std::cout << "Age: " << age << "\n";
        }
};
```

```cpp
Student stu;
stu.display();
```

这是允许的

```
Name: John
Age: 20
```

但如果直接函数中直接访问 `Person` 的 `name` 和 `age` 成员变量，就会报错

```cpp
Student stu;
std::cout << stu.name << "\n";
```

```
access.cpp:20:22: error: ‘std::string Person::name’ is protected within this context
```

## 对比图表

| 修饰符 | 类内部 | 派生类 | 类外部 |
| :---: | :---: | :---: | :---: |
| `public` | ✅ | ✅ | ✅ |
| `protected` | ✅ | ✅ | ❌ |
| `private` | ✅ | ❌ | ❌ |

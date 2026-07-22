# 构造函数

构造函数是类的成员函数，用于初始化对象

构造函数的名称与类名相同，没有返回类型，也不需要使用 `return` 关键字

## 语法

### 定义构造函数

```cpp
class 类名 {
    权限修饰符:
        类名(参数列表) // 构造函数
        {
            // 初始化代码
        }
};
```

- 构造函数的名称与**类名相同**
- 构造函数没有返回类型

### 调用构造函数

```cpp
类名 对象名(参数列表);
```

## 示例

现在有一个 `Student` 类

```cpp
class Student {
    public:
        std::string name;
        unsigned short age;
        double gpa;
};
```

假设要初始化一个 `Student` 对象，需要每次手动设置 `name`、`age` 和 `gpa`，非常麻烦

```cpp
Student student1;
student1.name = "Alice";
student1.age = 20;
student1.gpa = 3.5;

Student student2;
student2.name = "Bob";
student2.age = 22;
student2.gpa = 3.8;
```

这时我们可以使用构造函数来初始化对象

### 定义构造函数

```cpp
class Student {
    public:
        std::string name;
        unsigned short age;
        double gpa;

        // 构造函数
        Student(std::string name, unsigned short age, double gpa)
        {
            this->name = name;
            this->age = age;
            this->gpa = gpa;
        }
};
```

- `this`: 一个隐含的指针，指向**当前正在调用该成员函数的对象实例**
    - **作用**：在成员函数内部，用于区分“类的成员变量”和“局部变量（如函数参数）”
    - **示例**：在 `this->name = name;` 中，左边的 `name` 是对象的成员，右边的 `name` 是构造函数的参数
- `->`: 箭头运算符，用于通过指针访问对象或结构体的成员
    - `this->name` 等价于 `(*this).name`，即先解引用指针获得对象本身，再访问其成员

如果参数列表不使用与成员变量同名的参数，可以省略 `this` 关键字

```cpp
class Student {
    public:
        std::string name;
        unsigned short age;
        double gpa;

        // 构造函数
        Student(std::string x, unsigned short y, double z)
        {
            name = x;
            age = y;
            gpa = z;
        }
};
```

### 使用构造函数初始化对象

```cpp
Student student1("Alice", 20, 3.5);
```

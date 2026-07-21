# 结构体

结构体 (struct) 是一种用户自定义的数据类型，用于存储多个不同类型的变量

结构体中的变量可以是基本数据类型，也可以是其他结构体、数组或指针

## 语法

```cpp
struct 结构体名 {
    // 成员变量
    类型 成员名1;
    类型 成员名2;
    ...
};
```

## 示例

### 创建结构体

创建一个 `student` 结构体，包含 `name`、`age` 和 `gpa` 三个成员变量

```cpp
struct student {
    std::string name;
    unsigned short age;
    double gpa;
};
```

### 初始化结构体变量

创建一个 `student` 结构体变量 `student1`，并初始化其成员变量

```cpp
student student1 = {"Alice", 20, 3.5};
```

### 访问结构体成员

访问结构体成员变量需要使用成员访问运算符 `.`

```cpp
std::cout << "Name: " << student1.name << std::endl;
std::cout << "Age: " << student1.age << std::endl;
std::cout << "GPA: " << student1.gpa << std::endl;
```

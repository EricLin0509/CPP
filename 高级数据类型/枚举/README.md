# 枚举

枚举 (enum) 是一种特殊的数据类型，用于定义一组命名的常量

枚举常量通常表示某种特定的值或状态

## 语法

```cpp
enum 枚举名 {
    常量1 = 1, // 可以手动指定值
    常量2, // 那么接下来从2开始递增
    常量3,
    ...
};
```

- 每个枚举常量都代表一个**整数**，从0开始递增
- 默认情况下，第一个枚举常量的值为0
    - 也可以手动指定枚举常量的值，那么接下来的枚举常量的值也会自动递增
- 通常与 `switch-case` 语句结合使用

## 示例

使用枚举表示星期几

### 声明枚举

```cpp
enum Weekday {
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};
```

### 使用枚举

使用 `switch-case` 语句来表示枚举里每个常量

```cpp
Weekday day = Wednesday;

switch (day)
{
    case Monday:
        std::cout << "Today is Monday\n";
        break;
    case Tuesday:
        std::cout << "Today is Tuesday\n";
        break;
    case Wednesday:
        std::cout << "Today is Wednesday\n";
        break;
    case Thursday:
        std::cout << "Today is Thursday\n";
        break;
    case Friday:
        std::cout << "Today is Friday\n";
        break;
    case Saturday:
        std::cout << "Today is Saturday\n";
        break;
    case Sunday:
        std::cout << "Today is Sunday\n";
        break;
    default:
        std::cout << "Invalid day\n";
        break;
}
```

## 枚举的底层类型

枚举的底层类型是 `int`

我们可以在C语言中直接使用一个整数对枚举进行赋值 (**C++不允许这样写**)

```c
enum Weekday day = 8; // 这里超出枚举值范围
```

- 强烈不推荐使用这种方式进行赋值
    - 会导致代码难以维护和理解
    - 编译器无法进行类型检查
    - 破坏枚举的语义完整性
- 超出枚举值范围属于**未定义行为**
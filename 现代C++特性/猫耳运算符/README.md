# 猫耳运算符

猫耳运算符 (`^^`) 是指 C++26 中新增的反射运算符，因为外形像一个猫耳，所以被社区称为猫耳运算符

## 核心原理

在 C++26 标准中，官方正式引入了静态反射 (Static Reflection) 机制

C++26 的反射完全发生在编译期，具有零运行时开销（Zero Runtime Overhead）的特点

它主要依赖两个对称的机制：

1. 反射运算符 `^^` (Lift / 提升)：把代码中的实体**提升**为元数据对象，其返回类型为不透明的 `std::meta::info` 编译期常量
    - `std::meta::info`: 一个不透明的编译期类型，它可以表示类型、值、函数等各种 C++ 实体的元数据
2. 拼接运算符 `[: :]` (Splice / 剪接)：把元数据信息**还原**为实际的 C++ 代码 (类型、表达式、成员等)

## 示例

假设现在有一个枚举类型 `Color`，然后使用 `switch` 语句来处理不同的颜色值

```cpp
enum Color {
    Red,
    Green,
    Blue,
};
```

```cpp
Color c = Color::Red;

switch (c)
{
    case Color::Red:
        std::cout << "Red" << "\n";
        break;
    case Color::Green:
        std::cout << "Green" << "\n";
        break;
    case Color::Blue:
        std::cout << "Blue" << "\n";
        break;
    default:
        break;
}
```

但是这样会比较繁琐，如果枚举类型 `Color` 中有多个成员，可能会在 `switch` 中遗漏一些成员

那有什么更好的方法来处理这种情况吗？

### 传统方法——X-Macro

在 C++26 前，通常使用 X-Macro 来处理枚举类型的 `switch` 语句

```cpp
#define COLOR_LIST \
    X(Red) \
    X(Green) \
    X(Blue)

enum Color {
#define X(name) name,
    COLOR_LIST
};
#undef X
```

```cpp
Color c = Color::Red;
switch (c)
{
#define X(name) case Color::name: \
        std::cout << #name << "\n"; \
        break;
    COLOR_LIST
    default:
        break;
}
#undef X
```

- `#`: 字符串化运算符 (Stringizing Operator)，用于将宏参数转换为对应的字符串

此时如果要添加一个新的颜色值 `Yellow`，只需要在 `COLOR_LIST` 中添加 `X(Yellow)` 即可

```cpp
#define COLOR_LIST \
    X(Red) \
    X(Green) \
    X(Blue) \
    X(Yellow)
```

此时编译就会立马同步更新

但这样有个问题，就是代码非常反直觉

初学者看到 `COLOR_LIST` 配合反复的 `#define X` 和 `#undef X` 会直接懵掉，需要通过 `cpp` (C Preprocessor) 工具展开宏才能理解

同时 IDE 无法提供良好的代码提示，调试时也无法单步点进宏里面

### 痛点解决——静态反射

在 C++26 中，你就不需要改变写枚举的习惯

你就正常写最干净的枚举类型 `Color`，其余的事情交给反射去“扫描”

```cpp
enum Color {
    Red,
    Green,
    Blue,
    Yellow,
};
```

### 获取枚举的元数据

使用 `^^` 猫耳运算符来获取枚举类型的元数据对象

```cpp
constexpr std::meta::info color_info = ^^Color;
```

### 展开元数据对象

使用 `[: :]` 拼接运算符来展开元数据对象中的信息，并配合 `template for` 编译期循环语句来处理不同的颜色值

- `template for`: C++26 中配合反射引入的编译期循环语句，它会在编译期展开循环体
    - 这里是为每个枚举值生成一个 `case` 分支
    - 相比于传统的 `for` 循环，`template for` 可以在编译期展开循环体，避免了运行时的开销

```cpp
Color c = Color::Red;
switch (c)
{
    template for (const std::meta::info e : std::meta::enumerators_of(^^Color))
    {
        case [:e:]:
            std::cout << std::meta::name_of(e) << "\n";
            break;
    }
    default:
        break;
}
```

- `enumerators_of`: 获取枚举类型的所有枚举值元数据对象
- `name_of`: 获取枚举值的名称字符串

那编译器展开后等价于

```cpp
switch (c)
{
    case Color::Red:   std::cout << "Red"    << "\n"; break;
    case Color::Green: std::cout << "Green"  << "\n"; break;
    case Color::Blue:  std::cout << "Blue"   << "\n"; break;
    case Color::Yellow:std::cout << "Yellow" << "\n"; break;
    default: break;
}
```

## 与 X-Macro 对比

| 特征 | X-Macro | 静态反射 |
| :--: | :-----: | :-----: |
| 代码可读性 | 差，需要展开宏才能理解 | 好，代码直观易懂 |
| IDE 支持 | 差，无法提供良好的提示 | 好，IDE 可以正常分析 |
| 调试体验 | 差，无法单步进入宏 | 好，可以正常调试 |
| 维护成本 | 高，需要同步维护宏列表 | 低，添加枚举值即可自动生效 |
| 编译期安全 | 无，宏展开无类型检查 | 有，反射在编译期完成 |
| C++ 标准要求 | C++98 即可用 | C++26 |

## 总结

猫耳运算符 `^^` 和拼接运算符 `[: :]` 构成了 C++26 反射的核心对称机制：`^^` 把 C++ 实体提升为元数据，`[: :]` 把元数据还原为 C++ 代码

它们让 C++ 在编译期拥有了"自省"的能力，可以类型安全地替代各种宏技巧

简单来说就是

- C++ 实体 -> `^^` -> `std::meta::info` (元数据)
- C++ 实体 <- `[: :]` <- `std::meta::info` (元数据)

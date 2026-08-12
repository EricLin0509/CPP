# auto关键字

在C++中，`auto` 关键字用于自动推断变量的类型

它在编译时根据变量的初始化值或赋值来确定变量的类型

## 语法

```cpp
auto 变量名 = 初始值;
```

- 编译器在编译时根据变量的初始化值或赋值来确定变量的类型

## 示例

假设现在需要创建一个[共享智能指针](../../指针/智能指针/README.md#stdshared_ptr)变量

```cpp
std::shared_ptr<int> ptr = std::make_shared<int>(10);
```

现在创建额外的变量指向 `ptr`

```cpp
std::shared_ptr<int> ptr1 = ptr;
std::shared_ptr<int> ptr2 = ptr;
```

但是这样每次创建这个共享智能指针变量时，都需要写 `std::shared_ptr<int>` 这个类型

为了简化代码，可以使用 `auto` 关键字

### 使用 `auto` 关键字

```cpp
auto ptr = std::make_shared<int>(10);
auto ptr1 = ptr;
auto ptr2 = ptr;
```

这样，每次创建共享智能指针变量时，都不需要写 `std::shared_ptr<int>` 这个类型

# 关联容器

## 定义

关联容器是 C++ STL 库中的一种容器，它存储键值对，并且键是唯一的

## 关联容器类型

- [`map`](映射/README.md): 映射
- `multimap`: 多重映射
- `set`: 集合
- `multiset`: 多重集合

## 自定义比较函数

这里使用 `std::map` 作为示例，其他关联容器类型类似

可以使用自定义比较函数来改变容器中键的排序方式

这里需要重写 `operator()` (函数调用运算符)，该方法原型如下

```cpp
template <class Key, class Value>
bool operator()(const Key& x, const Value& y) const;
```

这里比较两个字符串长度

```cpp
struct CmpByLength {
    bool operator()(const std::string& x, const std::string& y) const {
        return x.length() < y.length();
    }
};
```

- 这里需要使用结构体包裹比较函数，因为比较函数参数模板需要一个类型参数

```cpp
std::map<std::string, int, CmpByLength> map;
```

### 使用 lambda 表达式

使用 lambda 表达式也可以实现比较函数

```cpp
auto cmp = [](const std::string& x, const std::string& y) {
    return x.length() < y.length();
}; // lambda 表达式

std::map<std::string, int, decltype(cmp)> map(cmp);
```

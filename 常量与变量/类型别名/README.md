# 类型别名

类型别名是一种语法糖，用于为现有类型创建一个新的名称

## 场景

假设现在我们有这样一个 `vector` 类型

```cpp
std::vector<std::pair<std::string, int>>
```

但如果我们经常需要使用这个类型，会导致代码冗长

## 使用 `typedef`

我们可以使用 `typedef` 来创建一个类型别名

```cpp
typedef std::vector<std::pair<std::string, int>> pairlist_t;
```

这样我们就可以使用 `pairlist_t` 来代替 `std::vector<std::pair<std::string, int>>`

```cpp
pairlist_t list;
```

## 使用 `using`

在 C++ 中，更推荐使用 `using` 来创建类型别名

```cpp
using pairlist_t = std::vector<std::pair<std::string, int>>;
```

```cpp
pairlist_t list;
```

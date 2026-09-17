# iota

`std::iota` 是C++11新增的一个数值算法，用于用从起始值开始连续递增的值填充一个范围

简单来说，它会把容器中的元素依次设为：`value, value+1, value+2, ...`

## 函数签名

```cpp
template< class ForwardIt, class T >
void iota( ForwardIt first, ForwardIt last, T value );
```

- `first`/`last`：要填充的范围的起始和结束迭代器
- `value`：要填充的起始值，后续的值会递增 (`++value`)

## 示例

### 声明一个 `vector`

```cpp
std::vector<int> vec(10);
```

### 使用 `iota` 填充 `vector`

```cpp
std::iota(vec.begin(), vec.end(), 0);
```

- 0: 从0开始递增
- 此时 `vec` 中的元素为：`0, 1, 2, 3, 4, 5, 6, 7, 8, 9`

## 常见用途

1. 生成索引序列: `std::iota(v.begin(), v.end(), 0);`
2. 配合排序使用：先 `iota` 生成索引，再按自定义规则对索引排序，实现"按值排序但保留原位置"的技巧

```cpp
std::vector<int> data = {4, 3, 2, 1};
std::vector<int> indices(data.size());
std::iota(indices.begin(), indices.end(), 0);
std::sort(indices.begin(), indices.end(), [&](int a, int b) {
    return data[a] < data[b];
});
for (int i : indices)
{
    std::cout << data[i] << " "; // 输出 1, 2, 3, 4
}
std::cout << "\n";
```

- 此时 `data` 数据保持不变，只是 `indices` 排序后指向的元素位置发生了变化

## 与 `std::fill` 的区别

| 算法 | 行为 |
| :--: | :--: |
| `iota` | 用从起始值开始连续递增的值填充范围 |
| `fill` | 用指定值填充范围 |

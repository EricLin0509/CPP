# 查找元素

现在有一个数组

```cpp
int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
```

现在要查找元素 `5`

这里我们使用线性查找算法

## 遍历数组

```cpp
int index = 0; // 存储元素的索引

for (index = 0; index < 10; index++) // 遍历数组
```

## 比较元素

```cpp
for (index = 0; index < 10; index++) // 遍历数组
{
    if (index == 5)
    {
        break; // 找到元素，退出循环
    }
}
```

## 处理失败情况

```cpp
if (index == 10) // 正常退出for循环时，index为10，表示元素不存在
{
    std::cout << "元素不存在" << std::endl; // 元素不存在
}
else
{
    std::cout << "元素存在，索引为" << index << std::endl; // 元素存在，索引为index
}
```

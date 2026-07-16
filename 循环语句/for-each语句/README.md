# for-each语句

for-each语句用于遍历可迭代容器中的元素，而不需要显式地使用索引

- 可迭代容器：如数组、vector、list、map等

它简化了遍历容器的代码，使得代码更加简洁易读

## 语法

```cpp
for (类型 变量 : 容器)
{
    // 循环体
}
```

## 示例

假设现在有一个数组

```cpp
int arr[] = {1, 2, 3, 4, 5};
```

要求遍历数组中的每个元素并输出

我们可以使用 `for` 语句

```cpp
for (int i = 0; i < 5; i++)
{
    std::cout << arr[i] << " ";
}
std::cout << "\n";
```

但这里可以使用 `for-each` 语句来简化代码

### 使用 for-each 语句

```cpp
for (int num : arr)
{
    std::cout << num << " ";
}
std::cout << "\n";
```


# return关键字

return关键字用于函数返回值，当函数执行到return语句时，函数会立即停止执行，并返回指定的值

## 语法

```cpp
return 返回值;
```

- **注意**: 如果使用 `void` 类型的函数，那么 `return` 语句后面**不能有返回值**
    - 通常情况下，在返回值类型为 `void` 的函数中使用 `return` 语句用于提前退出函数
    - 例如：检查数据是否有效，如果无效则提前退出函数

```cpp
void print(int *num)
{
    if (data == NULL)
    {
        cout << "Invalid data\n"
        return;
    }
    cout << *data << "\n";
}
```

- 返回值可以是任何类型的数据，包括基本数据类型、引用类型、指针类型等

## 示例

设计一个函数，计算两个数的和并返回结果

### 定义函数

```cpp
int add(int a, int b)
{
    return a + b;
}
```

### 调用函数

```cpp
int sum = add(3, 5);
```

- 如果返回值类型不为 `void`，那么一般会将返回值赋值给一个变量

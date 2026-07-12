# for语句

for语句用于在循环中执行一段代码，直到循环条件不再满足为止

## 语法

```cpp
for (初始化语句; 条件语句; 更新语句)
{
    // 循环体
}
```

## 示例

假设我们设计一个倒数程序，从10开始倒数到0

```cpp
int i = 10;
while (i >= 0)
{
    cout << i << "\n";
    i--;
}
cout << "Happy New Year\n";
```

我们可以用for语句来简化这段代码

### 使用 for 语句

```cpp
for (int i = 10; i >= 0; i--)
{
    cout << i << "\n";
}
cout << "Happy New Year\n";
```

如果现在我们要每隔2个数倒数，可以这样写

```cpp
for (int i = 10; i >= 0; i -= 2) // i -= 2 即每次循环减2
{
    cout << i << "\n";
}
cout << "Happy New Year\n";
```


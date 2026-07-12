# do-while语句

相比于while语句，do-while语句无论条件是否满足，都会**先执行一次循环体**，然后再判断条件是否满足

## 语法

```cpp
do
{
    // 代码块
} while (条件表达式);
```

## 示例

假设现在要设计一个获取一个正整数的程序

```cpp
int num = 0;

while (num < 0)
{
    cout << "请输入一个正整数：";
    cin >> num;
}
cout << "你输入的正整数是：" << num << endl;
```

但是这样的话，程序会直接跳过 `while` 循环体，直接输出 `num` 的值

我们可以在 `while` 前面添加一个 `cin` 语句，这样程序会先执行一次获取输入，然后再判断条件是否满足

```cpp
int num = 0;

cin >> num;

while (num < 0)
{
    cout << "请输入一个正整数：";
    cin >> num;
}
cout << "你输入的正整数是：" << num << endl;
```

但这样写会导致代码冗余

为此，我们可以使用 `do-while` 语句

### 使用 do-while 语句

```cpp
int num = 0;

do
{
    cout << "请输入一个正整数：";
    cin >> num;
} while (num < 0);
cout << "你输入的正整数是：" << num << "\n";
```

这样，程序会先执行一次获取输入，然后再判断条件是否满足，如果条件满足，程序会继续执行循环体，直到条件不满足为止

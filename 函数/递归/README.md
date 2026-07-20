# 递归函数

递归函数是函数调用自身的一种方式

递归函数通常用于解决需要重复计算的问题，它将一个复杂的问题分解为更小的子问题

- 优点
    - 简洁易读
    - 代码复用
- 缺点
    - 递归深度有限，容易导致栈溢出
    - 递归调用的次数过多，容易导致性能问题
- 与循环的区别
    - 最主要的区别是循环是从低到高推导，而递归是从高到低推导

## 语法

```cpp
返回值 函数名(参数列表)
{
    if(条件) // 终止条件
    {
        return <返回值>; // 如果返回值类型为void，可以省略返回值
    }
    else // 递归调用
    {
        函数名(参数列表);
    }
}
```

- 终止条件：当递归调用的参数满足某个条件时，函数不再调用自身，而是返回某个值
    - 没有终止条件，函数会无限递归调用，导致栈溢出
- 递归调用：当递归调用的参数不满足终止条件时，函数会调用自身，传递新的参数

## 示例

这里使用计算斐波那契数

```cpp
int fibonacci(int n);
```

### 设置终止条件

斐波那契数的第1项是1，第2项是1

所以终止条件是`n <= 2`

```cpp
int fibonacci(int n)
{
    if(n <= 2)
    {
        return 1;
    }
}
```

### 设置递归调用

斐波那契数的第n项是第n-1项和第n-2项的和

所以递归调用是`fibonacci(n-1) + fibonacci(n-2)`

```cpp
int fibonacci(int n)
{
    if(n <= 2)
    {
        return 1;
    }
    else
    {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}
```

### 调用函数

```cpp
int main()
{
    int n = 0;

    std::cout << "请输入一个正整数：";
    std::cin >> n;

    if (n <= 0)
    {
        std::cout << "请输入一个正整数！" << std::endl;
        return 1;
    }

    std::cout << "斐波那契数列的第" << n << "项是：" << fibonacci(n) << "\n"; // 调用函数

    return 0;
}
```

### 栈溢出

栈溢出 (Stack Overflow) 是递归函数最常见的问题之一

当递归调用的次数过多时，栈会溢出，导致程序崩溃

```cpp
void overflow(int n)
{
    std::cout << "Gen: " << n << " OK!\n";

    overflow(n + 1);
}
```

```cpp
overflow(1);
```

这样会一直递归调用，直到栈溢出

```
Gen: 261904 OK!
[1]    72110 segmentation fault (core dumped)  ./recursion
```

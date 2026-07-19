# 空指针

空指针是指指向内存地址为0的指针

在C++中，空指针通常用于表示一个变量没有被初始化或者一个指针没有指向任何有效的内存地址

## 语法

这里有两种写法

```cpp
int *ptr = nullptr; // C++写法

int *ptr = NULL; // C写法
```

其中 `NULL` 是C语言中的一个宏，用于表示空指针

```c
#define NULL (void *)0
```

## 示例

### 初始化

假设现在要初始化一个指针，但先不使用

可以先使用 `nullptr` 来初始化

```cpp
int *ptr = nullptr;
int a = 10;
ptr = &a; // 在此时进行赋值
```

### 检查是否为空

在使用指针之前，可以先检查它是否为空

```cpp
if (ptr == nullptr)
{
    // 指针为空
}
else
{
    // 指针不为空
}
```

### 实验：解引用空指针

解引用空指针属于未定义行为，会导致程序崩溃

```cpp
int *ptr = nullptr;
*ptr = 10; // 解引用空指针
```

此时会发生段错误 (Segmentation Fault)，程序崩溃

```
[1]    14430 segmentation fault (core dumped)  ./null_pointers
```
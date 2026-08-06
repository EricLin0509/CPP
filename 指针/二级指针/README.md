# 二级指针

二级指针是指指向指针的指针，通常用于动态分配内存或传递指针的指针。

## 语法

使用 `**` 符号来表示二级指针

```cpp
int *p;
int **q;
```

## 示例

假设有个一级指针 `p`，指向一个整数

```cpp
int a = 10;
int *p = &a;
```

可以通过一个二级指针 `q` 指向 `p`

```cpp
int **q = &p;
```

### 两者的地址

```cpp
std::cout << "Pointer p: " << p << "\n";
std::cout << "Pointer q: " << q << "\n";
std::cout << "Value of *q: " << *q << "\n";
```

```
Pointer p: 0x7fffffffd624
Pointer q: 0x7fffffffd628
Value of *q: 0x7fffffffd624
```

可以看到两者的地址不同，说明这是存储在不同的内存位置

且 `*q` 的值是 `p` 的地址，说明 `q` 指向 `p`

### 通过二级指针访问一级指针

```cpp
std::cout << "Value of *p: " << *p << "\n";
std::cout << "Value of **q: " << **q << "\n";
```

```
Value of *p: 10
Value of **q: 10
```

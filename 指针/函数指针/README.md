# 函数指针

函数指针是一种特殊的指针，它指向一个函数

通常在C语言中使用，C++也支持，但C++中更推荐使用 `std::function` 和 `std::bind` 来替代函数指针

## 语法

```cpp
返回值类型 (*指针名) (参数列表);
```

- **注意**: 函数指针的返回值类型和参数列表必须与函数的返回值类型和参数列表完全一致
    - 如果不匹配则编译器会报错

## 示例

设计两个函数，分别为 `c_to_f` 和 `f_to_c`，分别将摄氏度转换为华氏度和华氏度转换为摄氏度

```cpp
int c_to_f(int c)
{
    return c * 9 / 5 + 32;
}

int f_to_c(int f)
{
    return (f - 32) * 5 / 9;
}
```

现在想通过用户输入判断是摄氏度还是华氏度，然后调用相应的函数进行转换

```cpp
int value;
char choice;

std::cout << "Enter a temperature: ";
std::cin >> value;

std::cout << "Enter 'C' for Celsius or 'F' for Fahrenheit: ";
std::cin >> choice;
```

那么可以使用函数指针来实现回调函数

### 使用函数指针

```cpp
int (*converter)(int) = nullptr;

converter = (choice == 'C') ? c_to_f : f_to_c;

int result = converter(value);

std::cout << "The temperature is " << result << " degrees " << (choice == 'C' ? "Fahrenheit" : "Celsius") << ".\n";
```


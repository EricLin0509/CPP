# Hello World

此教程将介绍你的第一个C++程序——Hello World

## 编写代码

### 引入头文件

头文件是C++程序中包含的文件，用于定义类、函数、变量等。相当于应用依赖

```cpp
#include <iostream>
```

- iostream: Basic input-output stream library

### 定义main函数

main函数是C++程序的**逻辑入口点**

**注意事项**
- `main`函数的返回类型必须是`int`
- `main`函数不是程序的实际入口点，真正的入口点是 `_start` 函数：
    - C++运行时环境会先初始化`argc`（命令行参数数量）和`argv`（命令行参数数组），再调用`main`函数
    - 使用 `-nostartfiles` 编译选项时，必须手动定义 `_start` 函数

```cpp
int main()
{
    return 0; // 返回0表示程序正常结束
}
```

### 输出Hello World

使用`std::cout` 输出Hello World

```cpp
int main()
{
    std::cout << "Hello World!" << std::endl; // 输出Hello World并换行
    return 0;
}
```

- `std::cout` 是C++标准库中的**标准输出流对象**
- `<<` 是C++中的**流插入运算符**，用于将数据输出到输出流
- `std::endl` 是C++标准库中的**流操纵符**，执行换行操作并**刷新输出缓冲区**


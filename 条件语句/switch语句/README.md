# switch语句

switch语句用于根据条件执行不同的代码块

它通常与**枚举类型**一起使用，枚举类型用于定义一组常量，这些常量可以作为switch语句的条件

## 语法

```cpp
switch (变量名)
{
    case 常量1:
        // 当变量名等于常量1时执行的代码块
        break; // 可选
    case 常量2:
        // 当变量名等于常量2时执行的代码块
        break; // 可选
    ...
    case 常量n:
        // 当变量名等于常量n时执行的代码块
        break; // 可选
    default:
        // 当变量名不等于任何常量时执行的代码块
        break; // 可选
}
```

- 变量名：要比较的变量
- 常量：用于比较的常量 (可以是一个数字或者一个枚举类型)
- break：用于跳出switch语句，可选，不添则会继续执行下一个case块
- default：当变量名不等于任何常量时执行的代码块，可选
- **注意**：在switch语句中，每个case的值**必须是唯一**的，否则会报错

## 示例

假设现在我们获取用户输入判断是星期几

```cpp
using std::cout;
using std::cin;

int day = 0;
cout << "请输入星期几(1-7): ";
cin >> day;

if (day == 1)
{
    cout << "星期一" << "\n";
}
else if (day == 2)
{
    cout << "星期二" << "\n";
}
else if (day == 3)
{
    cout << "星期三" << "\n";
}
else if (day == 4)
{
    cout << "星期四" << "\n";
}
else if (day == 5)
{
    cout << "星期五" << "\n";
}
else if (day == 6)
{
    cout << "星期六" << "\n";
}
else if (day == 7)
{
    cout << "星期日" << "\n";
}
else
{
    cout << "输入错误" << "\n";
}
```

可以看到，如果使用 `if-else` 语句，代码会显得冗长且难以维护

为解决这个问题，我们可以使用 `switch` 语句

### 使用 `switch` 语句

```cpp
using std::cout;
using std::cin;

int day = 0;
cout << "请输入星期几(1-7): ";
cin >> day;

switch (day)
{
    case 1:
        cout << "星期一" << "\n";
        break;
    case 2:
        cout << "星期二" << "\n";
        break;
    case 3:
        cout << "星期三" << "\n";
        break;
    case 4:
        cout << "星期四" << "\n";
        break;
    case 5:
        cout << "星期五" << "\n";
        break;
    case 6:
        cout << "星期六" << "\n";
        break;
    case 7:
        cout << "星期日" << "\n";
        break;
    default:
        cout << "输入错误" << "\n";
        break;
}
```